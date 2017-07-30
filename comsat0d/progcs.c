/* progcs */

/* subroutine to read the messages coming in */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGN	0		/* special debug print-outs */
#define	CF_HDRDECODE	0		/* enable HDRDECODE */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The program was written from scratch to do what the previous program by
	the same name did.  It used pieces from other (similar in some ways)
	programs.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine reads in the COMAST messages and disposes of them
        appropriately.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<time.h>
#include	<stddef.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<char.h>
#include	<dispatcher.h>
#include	<localmisc.h>

#if	CF_DEBUGS || CF_DEBUG || CF_DEBUGN
#include	<debug.h>
#include	<upt.h>
#endif

#include	"config.h"
#include	"defs.h"
#include	"progcs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((2 * MAXPATHLEN),2048)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		100
#endif

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000		/* poll-time multiplier */
#endif

#define	NFDS		2
#define	PG_POLLREG	(POLLIN | POLLPRI | POLLERR)

#define	NDF		"progcs.deb"

#ifndef	TO_READ
#define	TO_READ		2
#endif


/* typedefs */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int,int *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getnprocessors(const char **,int) ;
extern int	mktmpuserdir(char *,const char *,const char *,mode_t) ;
extern int	msleep(int) ;
extern int	isasocket(int) ;

extern int	progexit(PROGINFO *) ;
extern int	progcsmsg(PROGINFO *,const char *,int) ;

extern int 	progerr_printf(PROGINFO *,const char *,...) ;

extern int 	prognote_check(PROGINFO *) ;

extern int	progloglock_begin(PROGINFO *) ;
extern int	progloglock_printf(PROGINFO *,const char *,...) ;
extern int	progloglock_end(PROGINFO *) ;
extern int	progloglock_maint(PROGINFO *) ;

extern int	strlinelen(const char *,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	progcs_begin(PROGINFO *,PROGCS *) ;
static int	progcs_end(PROGINFO *) ;
static int	progcs_hdrdecode(PROGINFO *) ;
static int	progcs_reader(PROGINFO *,DISPATCHER *) ;
static int	progcs_procmsg(PROGINFO *,DISPATCHER *,const char *,int) ;
static int	progcs_procmsger(PROGINFO *,DISPATCHER *,cchar *,int) ;
static int	progcs_worker(PROGINFO *,PROGCS_JOB *) ;


/* local variables */


/* exported subroutines */


int progcs(PROGINFO *pip)
{
	PROGCS		si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progcs/progcs: ent\n") ;
#endif

	if ((rs = progcs_begin(pip,sip)) >= 0) {
	    if ((rs = getnprocessors(pip->envv,0)) >= 0) {
	        DISPATCHER	d ;
	        const int	n = (rs+1) ;
	        void		*sub = (void *) progcs_worker ;
	        if ((rs = dispatcher_start(&d,n,sub,pip)) >= 0) {
		    int	f_abort ;

	            rs = progcs_reader(pip,&d) ;
		    c = rs ;
	            f_abort = (rs < 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progcs/progcs: progcs_reader() rs=%d\n",rs) ;
#endif

	            rs1 = dispatcher_finish(&d,f_abort) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (dispatcher) */
	    } /* end if (getnprocessors) */
	    rs1 = progcs_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progcs/progcs: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progcs) */


int progcs_trans(PROGINFO *pip,wchar_t *ibuf,int ilen,cchar *sp,int sl)
{
	PROGCS		*sip = pip->progcs ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if ((rs = progcs_hdrdecode(pip)) >= 0) {
	    if ((rs = ptm_lock(&sip->dm)) >= 0) {
	        HDRDECODE	*hdp = &sip->d ;
	        rs = hdrdecode_proc(hdp,ibuf,ilen,sp,sl) ;
	        len = rs ;
	        rs1 = ptm_unlock(&sip->dm) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (ptm) */
	} /* end if (progcs_hdrdecode) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (progcs_trans) */


/* local subroutines */


static int progcs_begin(PROGINFO *pip,PROGCS *sip)
{
	int		rs ;

	pip->progcs = sip ;
	memset(sip,0,sizeof(PROGCS)) ;
	sip->pip = pip ;
	rs = ids_load(&sip->id) ;

	return rs ;
}
/* end subroutine (progcs_begin) */


static int progcs_end(PROGINFO *pip)
{
	PROGCS		*sip = pip->progcs ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->open.hdrdecode) {
	    rs1 = ptm_destroy(&sip->dm) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = hdrdecode_finish(&sip->d) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = ids_release(&sip->id) ;
	if (rs >= 0) rs = rs1 ;

	pip->progcs = NULL ;
	return rs ;
}
/* end subroutine (progcs_end) */


static int progcs_hdrdecode(PROGINFO *pip)
{
	PROGCS		*sip = pip->progcs ;
	int		rs = SR_OK ;
	if (! sip->open.hdrdecode) {
	    cchar	*pr = pip->pr ;
	    if ((rs = hdrdecode_start(&sip->d,pr)) >= 0) {
		if ((rs = ptm_create(&sip->dm,NULL)) >= 0) {
		    sip->open.hdrdecode = TRUE ;
		}
		if (rs < 0) {
	    	    hdrdecode_finish(&sip->d) ;
		}
	    } /* end if (hdrdecode_start) */
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (progcs_begin) */


static int progcs_reader(PROGINFO *pip,DISPATCHER *dop)
{
	struct pollfd	fds[NFDS] ;
	time_t		ti_start = pip->daytime ;
	time_t		ti_termnote = pip->daytime ;
	time_t		ti_hangup = 0 ;
	const int	to_poll = TO_POLL ;
	const int	to = TO_READ ;	/* read time-out */
	const int	msglen = MSGBUFLEN ;
	const int	opts = FM_TIMED ;
	const int	fd_msg = pip->fd_msg ;
	int		rs = SR_OK ;
	int		intrun ;
	int		intnote ;
	int		nfds ;
	int		c = 0 ;
	int		f_eof = FALSE ;
	const char	*pn = pip->progname ;
	char		msgbuf[MSGBUFLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progcs/_reader: ent f_daemon=%u\n",pip->f.daemon) ;
	    debugprintf("progcs/_reader: intnote=%d\n",pip->intnote) ;
	    debugprintf("progcs/_reader: intidle=%d\n",pip->intidle) ;
	    debugprintf("progcs/_reader: intrun=%d\n",pip->intrun) ;
	}
#endif

	intnote = pip->intnote ;
	intrun = ((pip->f.daemon) ? pip->intrun : pip->intidle) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progcs/_reader: to_poll=%u to=%u\n",
		to_poll,to) ;
	    debugprintf("progcs/_reader: intrun=%d fd_msg=%d\n",
	        intrun,fd_msg) ;
	}
#endif

	nfds = 0 ;
	fds[nfds].fd = fd_msg ;
	fds[nfds].events = PG_POLLREG ;
	nfds += 1 ;
	fds[nfds].fd = -1 ;
	fds[nfds].events = 0 ;

	while (rs >= 0) {
	    const int	pollwait = (to_poll*POLLINTMULT) ;

#if	CF_DEBUG && 0
	    if (DEBUGLEVEL(4)) {
	        int	opts = FM_TIMED ;
	        rs1 = uc_recve(fd_msg,msgbuf,msglen,0,10,opts) ;
	        debugprintf("progcs/_reader: PRE uc_recve() rs=%d\n",rs1) ;
	        if (rs1 >= 0) {
	            debugprintf("progcs/_reader: PRE m=%>%t<\n",
	                msgbuf,strlinelen(msgbuf,rs1,30)) ;
	        }
	    }
#endif /* CF_DEBUG */

	    if ((rs = u_poll(fds,nfds,pollwait)) > 0) {
		int	i ;
	        pip->daytime = time(NULL) ;
	        for (i = 0 ; (rs >= 0) && (i < nfds) ; i += 1) {
		    const int	fd = fds[i].fd ;
	            const int	re = fds[i].revents ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            char	ebuf[EBUFLEN+1] ;
	            debugprintf("progcs/_reader: poll fd=%u re=%s\n",
	                fd,d_reventstr(re,ebuf,EBUFLEN)) ;
	        }
#endif /* CF_DEBUG */

	        if ((re & POLLIN) || (re & POLLPRI)) {
		    int	ml ;

	            msgbuf[0] = '\0' ;
	            if (pip->f.issocket) {
	                rs = uc_recve(fd,msgbuf,msglen,0,to,opts) ;
	                ml = rs ;
			f_eof = (pip->f.isstream && (rs == 0)) ;
	            } else {
	                rs = uc_reade(fd,msgbuf,msglen,to,opts) ;
	                ml = rs ;
			f_eof = (rs == 0) ;
	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progcs/_reader: read() rs=%d\n",rs) ;
#endif

	            if (pip->debuglevel > 0) {
	                progerr_printf(pip,"%s: recv(%d)\n",pn,rs) ;
		    }

	            if ((rs >= 0) && (ml > 0)) {

	                if (ml && (msgbuf[ml-1] == '\n')) ml -= 1 ;
	                while (ml && CHAR_ISWHITE(msgbuf[ml-1])) ml -= 1 ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("progcs/_reader: m=>%t<\n",
				msgbuf,ml) ;
			}
#endif

	                if (pip->debuglevel > 0) {
	                    progerr_printf(pip,"%s: msg=>%t<\n",
	                        pn,msgbuf,strlinelen(msgbuf,ml,40)) ;
			}

			c += 1 ;
	                rs = progcs_procmsg(pip,dop,msgbuf,ml) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	    			debugprintf("progcs/_reader: "
					"progcs_procmsg() rs=%d\n", rs) ;
			}
#endif

	            } /* end if (had non-zero-length message) */

	            if (rs == SR_TIMEDOUT) rs = SR_OK ;

	        } else if (re & POLLNVAL) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
		debugprintf("progcs/_reader: invalid?\n") ;
#endif
	            rs = SR_NOTOPEN ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLHUP) {
		    if (ti_hangup == 0) ti_hangup = pip->daytime ;
		    msleep(200) ; /* wait for remaining data */
	        } /* end if (poll-events) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progcs/_reader: inner-bot rs=%d f_eof=%u\n",
		        rs,f_eof) ;
#endif

		    if (f_eof) break ;
		} /* end for (file-descriptors) */
	    } else if (rs == 0) {
	        pip->daytime = time(NULL) ;
	    } else if (rs == SR_INTR) {
	        pip->daytime = time(NULL) ;
		rs = SR_OK ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
		debugprintf("progcs/_reader: mid rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && (intrun > 0)) {
	        if ((pip->daytime - ti_start) >= intrun) break ;
	    }

	    if ((rs >= 0) && ti_hangup) {
	        if ((pip->daytime - ti_hangup) >= 1) break ;
	    }

	    if ((rs >= 0) && pip->open.logprog) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progcs/_reader: "
				"progloglock_maint()\n") ;
#endif
		progloglock_maint(pip) ;
	    }

	    if ((rs >= 0) && ((pip->daytime - ti_termnote) >= intnote)) {
	        ti_termnote = pip->daytime ;
		rs = prognote_check(pip) ;
	    } /* end if (termnote_check) */

	    if (rs >= 0) rs = progexit(pip) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progcs/_reader: outer-bot rs=%d f_eof=%u\n",
		rs,f_eof) ;
#endif

	    if (f_eof) break ;
	} /* end while (looping) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcs/_reader: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progcs_reader) */


static int progcs_procmsg(PROGINFO *pip,DISPATCHER *dop,cchar *mbuf,int mlen)
{
	int		rs = SR_OK ;

	if (mbuf[0] != '\0') {
	    int		ml = strnlen(mbuf,mlen) ;
	    const char	*tp ;
	    const char	*mp = mbuf ;

	    while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	        rs = progcs_procmsger(pip,dop,mp,(tp-mp)) ;
	        pip->c_processed += 1 ;

		ml -= ((tp+1)-mp) ;
		mp = (tp+1) ;

		if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (ml > 0)) {
	        rs = progcs_procmsger(pip,dop,mp,ml) ;
	        pip->c_processed += 1 ;
	    }

	} /* end if (flie check) */

	return rs ;
}
/* end subroutine (progcs_procmsg) */


static int progcs_procmsger(PROGINFO *pip,DISPATCHER *dop,cchar *mp,int ml)
{
	PROGCS_JOB	*jp ;
	int		rs ;
	int		size = sizeof(PROGCS_JOB) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progcs_procmsger: ent m=>%t<\n",mp,ml) ;
#endif
	if (pip == NULL) return SR_FAULT ;
	if ((rs = uc_malloc(size,&jp)) >= 0) {
	    cchar	*cp ;
	    if ((rs = uc_mallocstrw(mp,ml,&cp)) >= 0) {
	        memset(jp,0,size) ;
	        jp->mp = cp ;
	        jp->ml = ml ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progcs_procmsger: m=>%t<\n",jp->mp,jp->ml) ;
#endif
	        rs = dispatcher_add(dop,jp) ;
		if (rs < 0) {
		    uc_free(cp) ;
		}
	    } /* end if (memory-allocation) */
	    if (rs < 0) {
		uc_free(jp) ;
	    }
	} /* end if (memory-allocation) */
	return rs ;
}
/* end subroutine (progcs_procmsger) */


/* this is a subroutine that runs in a parallel thread */
static int progcs_worker(PROGINFO *pip,PROGCS_JOB *jp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progcs_worker: ent\n") ;
	    debugprintf("progcs_worker: jp{%p}\n",jp) ;
	    debugprintf("progcs_worker: mp{%p} m=>%t<\n",
		jp->mp,jp->mp,jp->ml) ;
	}
#endif

	rs1 = progcsmsg(pip,jp->mp,jp->ml) ;
	if (rs >= 0) rs = rs1 ;

	if (jp != NULL) {
	    if (jp->mp != NULL) uc_free(jp->mp) ;
	    uc_free(jp) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    pthread_t	tid = pthread_self() ;
	    debugprintf("progcs_worker: tid=%u ret rs=%d\n",tid,rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (progcs_worker) */


