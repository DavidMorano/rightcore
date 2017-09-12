/* termnote */

/* perform terminal noticing */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* special debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        This object module was originally written to create a logging mechanism
        for PCS application programs.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a module that operates on termials (to be determined) for the
	purposes of putting out small messages to them.  Subroutines in this
	module are:

		termnote_open
		termnote_printf
		termnote_vprintf
		termnote_write
		termnote_close

	Implementation note:

        Yes, this can be written so that it performs in a more efficient manner.
        But this thing is quite seldom used right now (a-days). If there is ever
        a real need, yes, we can certaintly speed this up. So it is no big deal
        right now.

	BUG NOTE:

        Note that the (stupid) Solaris® version of the standard UTMPX interface
        does not work properly for some (unknown) strange reason. This bug is
        quite annoying since some terminals are not biffed as they should be due
        to the bug. The bug somehow causes an enumeration of the UTMPX database
        file to skip over some UTMPX records. Skipped records might just contain
        a terminal that needs to be biffed. To work around the Solaris® bug, we
        use a separate UTMPX database interface entirely (TMPX). That interface
        is not too dissimilar to the standard interface, except in the case of
        Solaris®, it works properly!


*******************************************************************************/


#define	TERMNOTE_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<ascii.h>
#include	<ids.h>
#include	<buffer.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<linefold.h>
#include	<tmpx.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"termnote.h"


/* local defines */

#define	TERMNOTE_SEARCHNAME	"termnote"
#define	TERMNOTE_DEBFNAME	"termnote.deb"

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#define	LOGDNAME	"log"

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif

#ifndef	TERMDEVLEN
#define	TERMDEVLEN	80
#endif

#define	TO_TMPX		(15 * 60)
#define	TO_CHECK	5
#define	TO_WRITE	30
#define	TO_LOGCHECK	5
#define	TO_LOCK		4

#define	USERTERM	struct userterm

#define	COLSTATE	struct colstate

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsdd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	ctdecui(char *,int,uint) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	getnodename(char *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	vbufprintf(char *,int,const char *,va_list) ;
extern int	termconseq(char *,int,int,int,int,int,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	writeto(int,const void *,int,int) ;
extern int	tmpx_getuserlines(TMPX *,VECSTR *,const char *) ;
extern int	charcols(int,int,int) ;
extern int	mkplogid(char *,int,const char *,int) ;
extern int	isprintlatin(int) ;
extern int	iceil(int,int) ;
extern int	ipow(int,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugprintf(const char *,...) ;
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwset(char *,int,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;

static int	vcmpatime(const void *,const void *) ;


/* external variables */


/* local structures */

struct userterm {
	time_t		atime ;
	char		termdev[MAXNAMELEN+1] ;
} ;

struct colstate {
	int		ncols ;
	int		ncol ;
} ;


/* forward references */

int		termnote_write(TERMNOTE *,const char **,int,int,
			const char *,int) ;
int		termnote_vprintf(TERMNOTE *,const char **,int,int,
			const char *,va_list) ;

static int	termnote_writer(TERMNOTE *,cchar **,int,int,cchar *,int) ;

static int	termnote_txopen(TERMNOTE *,time_t) ;
static int	termnote_txclose(TERMNOTE *) ;
static int	termnote_lfopen(TERMNOTE *,time_t) ;
static int	termnote_lfclose(TERMNOTE *) ;

static int	termnote_bufline(TERMNOTE *,BUFFER *,const char *,int) ;
static int	termnote_bufextra(TERMNOTE *,BUFFER *,int) ;
static int	termnote_dispose(TERMNOTE *,cchar **,int,int,cchar *,int) ;
static int	termnote_disposeuser(TERMNOTE *,int,int,cchar *,int,cchar *) ;
static int	termnote_disposewrite(TERMNOTE *,int,cchar *,int,cchar *) ;

static int	termnote_username(TERMNOTE *) ;
static int	termnote_nodename(TERMNOTE *) ;

#ifdef	COMMENT
static int	colstate_load(struct colstate *,int,int) ;
static int	colstate_linecols(struct colstate *,const char *,int) ;
#endif

static int	mkclean(char *,int,const char *,int) ;
static int	hasourbad(const char *,int) ;
static int	isourbad(int) ;


/* local variables */


/* exported subroutines */


int termnote_open(TERMNOTE *op,cchar *pr)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(TERMNOTE)) ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    op->pr = cp ;
	    if ((rs = termnote_txopen(op,dt)) >= 0) {
	        if ((rs = ids_load(&op->id)) >= 0) {
	    	    if ((rs = termnote_lfopen(op,dt)) >= 0) {
	                op->magic = TERMNOTE_MAGIC ;
	            }
	            if (rs < 0)
		        ids_release(&op->id) ;
	        } /* end if (ids_load) */
	        if (rs < 0)
		    termnote_txclose(op) ;
	    } /* end if (termnote_txopen) */
	    if (rs < 0) {
	        uc_free(op->pr) ;
	        op->pr = NULL ;
	    }
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (termnote_open) */


int termnote_close(TERMNOTE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMNOTE_MAGIC) return SR_NOTOPEN ;

	if (op->open.lf) {
	    time_t	dt = time(NULL) ;
	    char	tbuf[TIMEBUFLEN+1] ;
	    timestr_logz(dt,tbuf) ;
	    logfile_printf(&op->lf,"%s done",tbuf) ;
	}

	rs1 = termnote_lfclose(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->nodename != NULL) {
	    rs1 = uc_free(op->nodename) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nodename = NULL ;
	}

	rs1 = ids_release(&op->id) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = termnote_txclose(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	op->username[0] = '\0' ;
	op->magic = 0 ;
	return rs ;
}
/* end subroutine (termnote_close) */


/* make a log entry */
int termnote_printf(TERMNOTE *op,const char **rpp,int n,int o,cchar *fmt,...)
{
	int		rs ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = termnote_vprintf(op,rpp,n,o,fmt,ap) ;
	    va_end(ap) ;
	} /* end block */

	return rs ;
}
/* end subroutine (termnote_printf) */


/* make a log entry */
int termnote_vprintf(TERMNOTE *op,const char **rpp,int n,int o,
		cchar *fmt,va_list ap)
{
	const int	olen = TERMNOTE_BUFSIZE ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	char		*obuf ;

#if	CF_DEBUGS
	debugprintf("termnote_printf: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;
	if (ap == NULL) return SR_FAULT ;

	if (op->magic != TERMNOTE_MAGIC) return SR_BADF ;

	if ((rs = uc_malloc((olen+1),&obuf)) >= 0) {
	    if ((rs = vbufprintf(obuf,olen,fmt,ap)) >= 0) {
	        rs = termnote_write(op,rpp,n,o,obuf,rs) ;
	        wlen = rs ;
	    }
	    rs1 = uc_free(obuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a-f) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (termnote_vprintf) */


int termnote_check(TERMNOTE *op,time_t dt)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMNOTE_MAGIC) return SR_NOTOPEN ;

	if (dt == 0) dt = time(NULL) ;

	if ((dt - op->ti_check) >= TO_CHECK) {
	    op->ti_check = dt ;

	    if ((rs >= 0) && op->open.lf) {
	        if ((dt - op->ti_logcheck) >= TO_LOGCHECK) {
		    op->ti_logcheck = dt ;
	            rs = logfile_check(&op->lf,dt) ;
#if	CF_DEBUGN
		{
		const char	*dfname = TERMNOTE_DEBFNAME ;
		nprintf(dfname,"logfile_check() rs=%d\n",rs) ;
		}
#endif
		}
	    }

	    if ((rs >= 0) && op->open.tx) {
	        if ((dt - op->ti_tmpx) >= TO_TMPX) {
	            op->open.tx = FALSE ;
	            rs = tmpx_close(&op->tx) ;
	        } else
	            rs = tmpx_check(&op->tx,dt) ;
	    } /* end if (tmpx) */

	} /* end if (check) */

	return rs ;
}
/* end subroutine (termnote_check) */


int termnote_write(op,rpp,m,o,sbuf,slen)
TERMNOTE	*op ;
const char	**rpp ;
int		m ;
int		o ;
const char	sbuf[] ;
int		slen ;
{
	time_t		dt = 0 ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;
	if (sbuf == NULL) return SR_FAULT ;

	if (op->magic != TERMNOTE_MAGIC) return SR_NOTOPEN ;

	if (rpp[0] != NULL) {

	if (m < 1) m = INT_MAX ;

#if	CF_DEBUGS
	{
	    int	i ;
	    for (i = 0 ; rpp[i] != NULL ; i += 1) {
		debugprintf("termnote_write: r[%u]=%s\n",i,rpp[i]) ;
	    }
	    debugprintf("termnte_write: max=%d\n",m) ;
	    debugprintf("termnte_write: bell=%u biff=%u all=%u\n",
		((o & TERMNOTE_OBELL)?1:0),
		((o & TERMNOTE_OBIFF)?1:0),
		((o & TERMNOTE_OALL)?1:0)) ;
	}
#endif /* CF_DEBUGS */

	if ((rs >= 0) && op->open.lf) {
	    char	sublogid[LOGIDLEN+1] ;
	    char	tbuf[TIMEBUFLEN+1] ;
	    if ((rs = snsdd(sublogid,LOGIDLEN,op->logid,op->sn)) >= 0) {
		if ((rs = logfile_setid(&op->lf,sublogid)) >= 0) {
		    const int	f_bell = ((o & TERMNOTE_OBELL)?1:0) ;
		    const int	f_biff = ((o & TERMNOTE_OBIFF)?1:0) ;
		    const int	f_all = ((o & TERMNOTE_OALL)?1:0) ;
		    cchar		*fmt ;
		    if (dt == 0) dt = time(NULL) ;
		    timestr_logz(dt,tbuf),
		    fmt = "%s bell=%u biff=%u all=%u" ;
	    	    logfile_printf(&op->lf,fmt,tbuf,f_bell,f_biff,f_all) ;
		}
	    }
	} /* end if (logging) */

	if (slen < 0) slen = strlen(sbuf) ;

	if ((rs >= 0) && (slen > 0)) {

#if	CF_DEBUGS
	debugprintf("termnote_write: note=>%t<\n",
		sbuf,strlinelen(sbuf,slen,40)) ;
#endif

/* ok, now we really go */

	    rs = termnote_writer(op,rpp,m,o,sbuf,slen) ;
	    c = rs ;

	    if ((rs >= 0) && op->open.lf) {
	        int	n = (LOGIDLEN - 1 - strlen(op->logid)) ;
	        int	m ;
	        m = (n < 10) ? ipow(10,n) : INT_MAX ;
	        op->sn = ((op->sn + 1) % m) ;
	        rs = logfile_setid(&op->lf,op->logid) ;
	    }

	    if (rs >= 0) {
	        if (dt == 0) dt = time(NULL) ;
	        op->ti_write = dt ;
	    }

	} /* end if (positive) */

	} /* end if (not-empty) */

#if	CF_DEBUGS
	debugprintf("termnote_write: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (termnote_write) */


/* private subroutines */


static int termnote_writer(TERMNOTE *op,cchar **rpp,int m,int o,
		cchar *sp,int sl)
{
	BUFFER		ob ;
	const int	bsize = (sl + 40) ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = buffer_start(&ob,bsize)) >= 0) {
	    const int	maxlines = TERMNOTE_MAXLINES ;
	    int		lines = 0 ;
	    cchar	*tp ;

	    while ((tp = strnchr(sp,sl,'\n')) != NULL) {
		if (lines < maxlines) {
	            rs = termnote_bufline(op,&ob,sp,(tp - sp)) ;
		    lines += 1 ;
		}
	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;
	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (sl > 0)) {
		if (lines < maxlines) {
	             rs = termnote_bufline(op,&ob,sp,sl) ;
		     lines += 1 ;
		}
	    }

	    if (rs >= 0)
		rs = termnote_bufextra(op,&ob,o) ;

	    if (rs >= 0) {
	        const char	*bp ;
	        int		bl ;
	        if ((rs = buffer_get(&ob,&bp)) >= 0) {
	            bl = rs ;
		    if (op->open.lf) {
			cchar	*fmt = "note lines=%u len=%u" ;
			logfile_printf(&op->lf,fmt,lines,bl) ;
		    }
		    rs = termnote_dispose(op,rpp,m,o,bp,bl) ;
		    c = rs ;
		} /* end if (get) */
	    } /* end if */

	    rs1 = buffer_finish(&ob) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (termnote_writer) */


static int termnote_bufline(TERMNOTE *op,BUFFER *obp,cchar *lp,int ll)
{
	const int	cols = COLUMNS ;
	const int	tmplen = COLUMNS ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (lp == NULL) return SR_FAULT ;

	if (ll < 0) ll = strlen(lp) ;

	if (lp[ll-1] == '\n') ll -= 1 ;

#if	CF_DEBUGS
	debugprintf("termnote_bufline: line=>%t<\n",
		lp,strlinelen(lp,ll,40)) ;
#endif

	if (ll > 0) {
	    LINEFOLD	lf ;
	    if ((rs = linefold_start(&lf,cols,1,lp,ll)) >= 0) {
	        int		i ;
		int		cl ;
		int		bl ;
		const char	*bp ;
		const char	*cp ;
	        char	tmpbuf[COLUMNS+ 1] ;
	        for (i = 0 ; (cl = linefold_get(&lf,i,&cp)) >= 0 ; i += 1) {
	            bp = cp ;
	            bl = cl ;
	            if (hasourbad(cp,cl)) {
	                bp = tmpbuf ;
	                bl = mkclean(tmpbuf,tmplen,cp,cl) ;
	            }
	            rs = buffer_char(obp,'\r') ;
		    if (rs >= 0)
	                rs = buffer_strw(obp,bp,bl) ; /* releases 'tmpbuf' */
		    if (rs >= 0) {
		         int	tl ;
		         rs = termconseq(tmpbuf,(tmplen-2),'K',-1,-1,-1,-1) ;
		         tl = rs ;
		         if (rs >= 0) {
			    tmpbuf[tl++] = '\r' ;
			    tmpbuf[tl++] = '\n' ;
	        	    rs = buffer_strw(obp,tmpbuf,tl) ;
		         }
		    }
	            if (rs < 0) break ;
	        } /* end for (getting folded line parts) */
	        rs1 = linefold_finish(&lf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (linefold) */
	} /* end if (non-zero) */

#if	CF_DEBUGS
	debugprintf("termnote_bufline: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (termnote_bufline) */


static int termnote_bufextra(TERMNOTE *op,BUFFER *obp,int o)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (o & TERMNOTE_OBELL) {
	    rs = buffer_char(obp,CH_BELL) ;
	}

	return rs ;
}
/* end subroutine (termnote_bufextra) */


static int termnote_dispose(TERMNOTE *op,cchar **rpp,int n,int o,
		cchar *bp,int bl)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("termnote_dispose: ent\n") ;
#endif

	for (i = 0 ; (rs >= 0) && (rpp[i] != NULL) ; i += 1) {
	    rs = termnote_disposeuser(op,n,o,bp,bl,rpp[i]) ;
	    c += rs ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("termnote_dispose: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (termnote_dispose) */


static int termnote_disposeuser(op,max,o,bp,bl,un)
TERMNOTE	*op ;
int		max ;
int		o ;
const char	*bp ;
int		bl ;
const char	un[] ;
{
	VECOBJ		uts ;
	const int	utsize = sizeof(USERTERM) ;
	const int	tdlen = TERMDEVLEN ;
	int		rs ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	const char	*devdname = DEVDNAME ;
	char		termfname[TERMDEVLEN+1] ;

#if	CF_DEBUGS
	debugprintf("termnote_disposeuser: u=%s\n",un) ;
#endif

	if ((rs = vecobj_start(&uts,utsize,0,0)) >= 0) {
	    VECSTR	lines ;

	    if ((rs = vecstr_start(&lines,0,0)) >= 0) {
	        int	nlines ;

		rs = tmpx_getuserlines(&op->tx,&lines,un) ;
		nlines = rs ;

#if	CF_DEBUGS
		debugprintf("termnote_disposeuser: tmpx_getuserlines() rs=%d\n",
			rs) ;
		{
		    const char	*cp ;
		    for (i = 0 ; vecstr_get(&lines,i,&cp) >= 0 ; i += 1)
			debugprintf("termnote_disposeuser: termline=%s\n",cp) ;
		}
#endif /* CF_DEBUGS */

		if (op->open.lf)
		    logfile_printf(&op->lf,"u=%s termlines=%u",un,nlines) ;

	    if ((rs >= 0) && (nlines > 0)) {
		USERTERM	ut ;
		int		ll, tl ;
		const char	*lp ;

		for (i = 0 ; vecstr_get(&lines,i,&lp) >= 0 ; i += 1) {
		    if (lp == NULL) continue ;
		    ll = strlen(lp) ;

#if	CF_DEBUGS
			debugprintf("termnote_disposeuser: 2 termline=%s\n",
				lp) ;
#endif
		    if (ll > 0) {
			rs = mkpath2w(termfname,devdname,lp,ll) ;
			tl = rs ;

#if	CF_DEBUGS
			debugprintf("termnote_disposeuser: termdev=%s\n",
			    termfname) ;
#endif
			if (rs >= 0) {
			    rs1 = sncpy1w(ut.termdev,tdlen,termfname,tl) ;
			    if (rs1 >= 0) {
				struct ustat	sb ;
				int		f_go = FALSE ;
				rs1 = u_stat(ut.termdev,&sb) ;
#if	CF_DEBUGS
				debugprintf("termnote_disposeuser: "
					"u_stat() rs=%d\n",rs1) ;
				debugprintf("termnote_disposeuser: "
					"mode=%04o\n",(sb.st_mode & S_IAMB)) ;
#endif
				if (rs1 >= 0) {
				    f_go = (sb.st_mode & S_IWGRP) ;
				    if (f_go && (o & TERMNOTE_OBIFF))
				        f_go = (sb.st_mode & S_IXUSR) ;
				}
				if ((rs1 >= 0) && f_go) {
				    rs1 = sperm(&op->id,&sb,W_OK) ;
#if	CF_DEBUGS
				debugprintf("termnote_disposeuser: "
					"sperm() rs=%d\n",rs1) ;
#endif
				    if (rs1 >= 0) {
					ut.atime = sb.st_atime ;
					rs = vecobj_add(&uts,&ut) ;
				    }
				}
			    }
			}
		    } /* end if (positive) */

		    if (rs < 0) break ;
		} /* end for */

	    } /* end if */

		rs1 = vecstr_finish(&lines) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (user-term lines) */

#if	CF_DEBUGS
	    debugprintf("termnote_disposeuser: mid rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        USERTERM	*utp ;
		int		navail ;
		int		n = 0 ;

		navail = vecobj_sort(&uts,vcmpatime) ;

		if (op->open.lf)
		    logfile_printf(&op->lf,"  avail=%u",navail) ;

		for (i = 0 ; vecobj_get(&uts,i,&utp) >= 0 ; i += 1) {
		    if (utp == NULL) continue ;
		    rs1 = termnote_disposewrite(op,o,bp,bl,utp->termdev) ;
#if	CF_DEBUGS
	    	    debugprintf("termnote_disposeuser: termdev=%s\n",
			utp->termdev) ;
	    	    debugprintf("termnote_disposeuser: "
				"_disposewrite() rs=%d\n",rs) ;
#endif
		    if (rs1 > 0) {
			n += 1 ;
			c += 1 ;
		    }

		if (op->open.lf)
		    logfile_printf(&op->lf,"  %s (%d)",
			(utp->termdev+5),rs1) ;

		    if (n >= max) break ;
		    if (rs < 0) break ;
		} /* end for (looping through user-terms) */

	    } /* end if (ok) */

	    rs1 = vecobj_finish(&uts) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (user-term list) */

#if	CF_DEBUGS
	debugprintf("termnote_disposeuser: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (termnote_disposeuser) */


/* ARGSUSED */
static int termnote_disposewrite(op,o,bp,bl,termdev)
TERMNOTE	*op ;
int		o ;
const char	*bp ;
int		bl ;
const char	termdev[] ;
{
	const int	of = (O_WRONLY | O_NOCTTY | O_NDELAY) ;
	const int	to = 5 ;
	int		rs ;
	int		len = 0 ;

	if ((rs = u_open(termdev,of,0666)) >= 0) {
	    const int	fd = rs ;
	    rs = writeto(fd,bp,bl,to) ;
	    len = rs ;
	    u_close(fd) ;
	} /* end if (open terminal-device) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termnote_disposewrite) */


static int termnote_txopen(TERMNOTE *op,time_t dt)
{
	int		rs = SR_OK ;

	if (! op->open.tx) {
	    if (dt == 0) dt = time(NULL) ;
	    rs = tmpx_open(&op->tx,NULL,0) ;
	    op->open.tx = (rs >= 0) ;
	    if (rs >= 0) op->ti_tmpx = dt ;
	}

	return rs ;
}
/* end subroutine (termnote_txopen) */


static int termnote_txclose(TERMNOTE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->open.tx) {
	    op->open.tx = FALSE ;
	    rs1 = tmpx_close(&op->tx) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (termnote_txclose) */


static int termnote_lfopen(TERMNOTE *op,time_t dt)
{
	const mode_t	om = 0666 ;
	const int	of = O_RDWR ;
	int		rs = SR_OK ;
	int		f_opened = FALSE ;

	if (! op->init.lf) {
	    const char	*sn = TERMNOTE_SEARCHNAME ;
	    char	lfname[MAXPATHLEN+1] ;
	    op->init.lf = TRUE ;
	    if (rs >= 0) {
		rs = termnote_nodename(op) ;
	    }
	    if (rs >= 0) {
		const pid_t	pid = ugetpid() ;
		rs = mkplogid(op->logid,LOGIDLEN,op->nodename,pid) ;
	    }
	    if (rs >= 0) {
	        rs = mkpath3(lfname,op->pr,LOGDNAME,sn) ;
	    }
	    if (rs >= 0) {
		LOGFILE	*lfp = &op->lf ;
	        if ((rs = logfile_open(lfp,lfname,of,om,op->logid)) >= 0) {
		    f_opened = TRUE ;
	            op->open.lf = TRUE ;
#if	CF_DEBUGS
		debugprintf("termnote_lfopen: logfile_open() rs=%d\n",rs1) ;
#endif
#if	CF_DEBUGN
		{
		const char	*dfname = TERMNOTE_DEBFNAME ;
		nprintf(dfname,"logfname=%s\n",lfname) ;
		nprintf(dfname,"logfile_open() rs=%d\n",rs1) ;
		}
#endif
		    if (rs >= 0) {
		        rs = logfile_checksize(&op->lf,TERMNOTE_LOGSIZE) ;
		    }
		    if (rs >= 0) {
		        rs = termnote_username(op) ;
		    }
		    if (rs >= 0) {
			cchar	*nn = op->nodename ;
			cchar	*un = op->username ;
			char	timebuf[TIMEBUFLEN+1] ;
	        	if (dt == 0) dt = time(NULL) ;
			timestr_logz(dt,timebuf) ;
			logfile_printf(&op->lf,"%s %s",timebuf,sn) ;
			rs = logfile_printf(&op->lf,"%s!%s",nn,un) ;
#if	CF_DEBUGS
			debugprintf("termnote_lfopen: "
				"logfile_printf() rs=%d\n",rs1) ;
#endif
		    }
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
		} /* end if (logfile opened) */
	    } /* end if (ok) */
	} /* end if (needed initialization) */

#if	CF_DEBUGS
	debugprintf("termnote_lfopen: ret rs=%d f_open=%u\n",rs,f_opened) ;
#endif

	return (rs >= 0) ? f_opened : rs ;
}
/* end subroutine (termnote_lfopen) */


static int termnote_lfclose(TERMNOTE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->open.lf) {
	    op->open.lf = FALSE ;
	    rs1 = logfile_close(&op->lf) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (termnote_lfclose) */


static int termnote_username(TERMNOTE *op)
{
	int		rs = SR_OK ;
	int		ul ;

	if (op->username[0] == '\0') {
	   rs = getusername(op->username,USERNAMELEN,-1) ;
	   ul = rs ;
	} else
	   ul = strlen(op->username) ;

	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (termnote_username) */


static int termnote_nodename(TERMNOTE *op)
{
	int		rs = SR_OK ;
	int		nl = 0 ;

	if (op->nodename == NULL) {
	    char	nn[NODENAMELEN+1] ;
	    const char	*np ;
	    if ((rs = getnodename(nn,NODENAMELEN)) >= 0) {
	        nl = rs ;
		rs = uc_mallocstrw(nn,nl,&np) ;
		if (rs >= 0) op->nodename = np ;
	    }
	} else
	    nl = strlen(op->nodename) ;

	return (rs >= 0) ? nl : rs ;
}
/* end subroutine (termnote_nodename) */


#ifdef	COMMENT

static int colstate_load(csp,ncols,ncol)
struct colstate	*csp ;
int		ncols ;
int		ncol ;
{

	csp->ncols = ncols ;
	csp->ncol = ncol ;
	return SR_OK ;
}
/* end subroutine (colstate_load) */


/* return the number of characters that will fill the current column limit */
static int colstate_linecols(csp,sbuf,slen)
struct colstate	*csp ;
const char	sbuf[] ;
int		slen ;
{
	int		i ;
	int		cols ;
	int		rcols ;

	rcols = (csp->ncols - csp->ncol) ;
	for (i = 0 ; (rcols > 0) && (i < slen) ; i += 1) {

	    cols = charcols(NTABCOLS,csp->ncol,sbuf[i]) ;

	    if (cols > rcols)
	        break ;

	    csp->ncol += cols ;
	    rcols -= cols ;

	} /* end for */

	return i ;
}
/* end subroutine (colstate_linecols) */

#endif /* COMMENT */


static int mkclean(char *outbuf,int outlen,cchar *sbuf,int slen)
{
	int		i ;
	for (i = 0 ; (i < outlen) && (i < slen) ; i += 1) {
	    outbuf[i] = sbuf[i] ;
	    if (isourbad(sbuf[i] & 0xff)) outbuf[i] = '­' ;
	} /* end for */
	return i ;
}
/* end subroutine (mkclean) */


static int hasourbad(cchar *sp,int sl)
{
	int		ch ;
	int		f = FALSE ;
	while (sl && (sp[0] != '\0')) {
	    ch = (sp[0] & 0xff) ;
	    f = isourbad(ch) ;
	    if (f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end if */
	return f ;
}
/* end subroutine (hasourbad) */


static int isourbad(int ch)
{
	int		f ;
	switch (ch) {
	case CH_SO:
	case CH_SI:
	case CH_SS2:
	case CH_SS3:
	case '\t':
	    f = FALSE ;
	    break ;
	default:
	    f = (! isprintlatin(ch)) ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isourbad) */


/* we want to do a reverse sort here (in descending order) */
static int vcmpatime(const void *v1pp,const void *v2pp)
{
	USERTERM	**e1pp = (USERTERM **) v1pp ;
	USERTERM	**e2pp = (USERTERM **) v2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = (*e2pp)->atime - (*e1pp)->atime ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	} 
	return rc ;
}
/* end subroutine (vcmpatime) */


