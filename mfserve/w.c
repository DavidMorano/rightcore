/* mfswatch */

/* watch (listen on) the specified service-access-points */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	1		/* switchable debug print-outs */
#define	CF_SVC		1		/* code the service DB stuff */


/* revision history:

	= 2008-06-23, David A­D­ Morano
        I updated this subroutine to just poll for machine status and write the
        Machine Status (MS) file. This was a cheap excuse for not writing a
        whole new daemon program just to poll for machine status. I hope this
        works out! :-)

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is responsible for listening on the given socket and
        spawning off a program to handle any incoming connection. Some of the
        "internal" messages are handled here (the easy ones -- or the ones that
        fit here best). The rest (that look like client-sort-of requests) are
        handled in the 'standing' object module.

	Notes:

	+ job types:
		0	jobtype_req
		1	jobtype_listen

	+ listen types (sub-type):
		0	"none"
		1	"tcp"
		2	"uss"
		3	"pass"


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<sockaddress.h>
#include	<connection.h>
#include	<poller.h>
#include	<svcfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"sreqdb.h"
#include	"listenspec.h"
#include	"defs.h"
#include	"proglog.h"
#include	"mfsmain.h"
#include	"mfslocinfo.h"
#include	"mfsconfig.h"
#include	"mfslisten.h"
#include	"mfsadj.h"
#include	"mfslog.h"
#include	"mfsmsg.h"
#include	"mfsbuilt.h"
#include	"svcentsub.h"


/* local defines */

#define	MFSWATCH	struct mfswatch
#define	MFSWATCH_FL	struct mfswatch_flags

#ifndef	IPCDIRMODE
#define	IPCDIRMODE	0777
#endif

#define	W_OPTIONS	(WNOHANG)

#define	IPCBUFLEN	MSGBUFLEN

#ifndef	CMSGBUFLEN
#define	CMSGBUFLEN	256
#endif

#define	SBUFLEN		LINEBUFLEN

#define	O_SRVFLAGS	(O_RDWR | O_CREAT)

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	TO_LOGFLUSH
#define	TO_LOGFLUSH	10
#endif


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;
extern int	snsdd(char *,int,const char *,uint) ;
extern int	snpollflags(char *,int,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	bufprintf(const char *,int,...) ;
extern int	dupup(int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	nlspeername(const char *,const char *,char *) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	acceptpass(int,struct strrecvfd *,int) ;
extern int	varsub_addvec(VARSUB *,VECSTR *) ;
extern int	vecstr_svcargs(vecstr *,cchar *) ;
extern int	isasocket(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isBadMsg(int) ;
extern int	isBadSend(int) ;

#if	CF_DEBUGS || CF_DEBUG 
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
extern int	progexports(PROGINFO *,const char *) ;
#endif /* CF_DEBUGS */

extern cchar	*strsigabbr(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

struct mfswatch_flags {
	uint		done:1 ;	/* exit ASAP */
	uint		sdb:1 ;
} ;

struct mfswatch {
	MFSWATCH_FL	f, open ;
	SREQDB		reqs ;		/* service requests */
	POLLER		pm ;		/* Poll-Manager */
	MFSBUILT	built ;		/* service lookup database */
	SVCFILE		sdb ;		/* service lookup database */
	time_t		ti_lastmark ;
	time_t		ti_lastmaint ;
	time_t		ti_lastconfig ;
	int		njobs ;
	int		minpoll ;	/* poll interval in milliseconds */
} ;


/* forward references */

static int	mfswatch_beginner(PROGINFO *) ;
static int	mfswatch_ender(PROGINFO *) ;
static int	mfswatch_polljobs(PROGINFO *,int,int) ;
static int	mfswatch_svcaccum(PROGINFO *,SREQ *,int,int) ;
static int	mfswatch_uptimer(PROGINFO *) ;
static int	mfswatch_poll(PROGINFO *,POLLER_SPEC *) ;
static int	mfswatch_pollreg(PROGINFO *,int,int) ;

static int	mfswatch_svcbegin(PROGINFO *) ;
static int	mfswatch_svcend(PROGINFO *) ;

#if	CF_SVC
static int	mfswatch_svcfind(PROGINFO *,SREQ *) ;
static int	mfswatch_svcfinder(PROGINFO *,SREQ *,vecstr *) ;
static int	mfswatch_svcproc(PROGINFO *,SREQ *,SVCFILE_ENT *,vecstr *) ;
static int	mfswatch_notfound(PROGINFO *,SREQ *) ;
static int	mfswatch_jobretire(PROGINFO *,SREQ *) ;
#endif /* CF_SVC */

#if	CF_SVC && defined(COMMENT)
static int	mfswatch_islong(PROGINFO *,vecstr *) ;
#endif


/* local variables */


/* exported subroutines */


int mfswatch_begin(PROGINFO *pip)
{
	int		rs = SR_BUGCHECK ;
	if (pip->watch == NULL) {
	    const int	osize = sizeof(MFSWATCH) ;
	    void	*p ;
	    if ((rs = uc_malloc(osize,&p)) >= 0) {
	        pip->watch = p ;
		memset(p,0,osize) ;
	        if ((rs = mfswatch_beginner(pip)) >= 0) {
		    pip->open.watch = TRUE ;
	        }
		if (rs < 0) {
		    uc_free(pip->watch) ;
		    pip->watch = NULL ;
		}
	    } /* end if (m-a) */
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (mfswatch_begin) */


int mfswatch_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->watch != NULL) {
	    if (pip->open.watch) {
	        pip->open.watch = FALSE ;
	        rs1 = mfswatch_ender(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(pip->watch) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->watch = NULL ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_end: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_end) */


int mfswatch_service(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_service: ent\n") ;
#endif

	if ((rs = mfswatch_uptimer(pip)) >= 0) {
	    POLLER	*pmp = &wip->pm ;
	    POLLER_SPEC	ps ;

	    if ((rs = poller_wait(pmp,&ps,wip->minpoll)) > 0) {
	        pip->daytime = time(NULL) ;
	        if ((rs = mfswatch_poll(pip,&ps)) >= 0) {
		    while ((rs = poller_get(pmp,&ps)) > 0) {
	    	        rs = mfswatch_poll(pip,&ps) ;
		        if (rs < 0) break ;
		    } /* end while */
	        } /* end if */
	    } else if (rs == 0) {
	        pip->daytime = time(NULL) ;
	    } else if (rs == SR_INTR) {
	        pip->daytime = time(NULL) ;
	        rs = SR_OK ;
	    } /* end if */

	    if (rs >= 0) {
	       const int	to = TO_MAINT ;
	       if ((pip->daytime-wip->ti_lastmaint) >= to) {
		    wip->ti_lastmaint = pip->daytime ;
		    rs = mfslisten_maint(pip,pmp) ;
	       }
	    }

	    if ((rs >= 0) && (pip->config != NULL)) {
	       const int	to = TO_CONFIG ;
	       if ((pip->daytime-wip->ti_lastconfig) >= to) {
		    CONFIG	*cfp = pip->config ;
		    wip->ti_lastconfig = pip->daytime ;
		    rs = config_check(cfp) ;
	       }
	    }

	} /* end if (mfswatch_uptimer) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_service: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfswatch_service) */


/* local subroutines */


static int mfswatch_uptimer(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;

	if (wip->njobs <= 0) {
	    if (wip->minpoll < 100) {
		wip->minpoll = 100 ;
	    }
	    wip->minpoll += 100 ;
	} else {
	    wip->minpoll += 10 ;
	}

	if (wip->minpoll < 10) {
	    wip->minpoll = 10 ;
	} else if (wip->minpoll > POLLINTMULT) {
	    wip->minpoll = POLLINTMULT ;
	}

	return SR_OK ;
}
/* end subroutine (mfswatch_uptimer) */


/* poll (everything) for a hit */
static int mfswatch_poll(PROGINFO *pip,POLLER_SPEC *psp)
{
	MFSWATCH	*wip = pip->watch ;
	POLLER		*pmp ;
	const int	fd = psp->fd ;
	const int	re = psp->revents ;
	int		rs ;
	int		f = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    const int	plen = TIMEBUFLEN ;
	    char	pbuf[TIMEBUFLEN+1] ;
	    snpollflags(pbuf,plen,re) ;
	    debugprintf("mfswatch_poll: fd=%u re=(%s)\n",fd,pbuf) ;
	}
#endif /* CF_DEBUG */

	pmp = &wip->pm ;
	if ((rs = mfsadj_poll(pip,pmp,fd,re)) > 0) {
	    f = TRUE ;
	} else if (rs == 0) {
	    if ((rs = mfslisten_poll(pip,pmp,fd,re)) > 0) {
	        f = TRUE ;
	    } else if (rs == 0) {
		if ((rs = mfswatch_polljobs(pip,fd,re)) > 0) {
	            f = TRUE ;
		} else if (rs == 0) {
#if	CF_DEBUG
		    if (DEBUGLEVEL(4))
	    	    debugprintf("mfswatch_poll: zero\n") ;
#endif
		}
	    }
	    if (rs > 0) {
		logflush(pip) ;
	    }
	} /* end if (our intra-program message portal) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_poll: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfswatch_poll) */


int mfswatch_newjob(PROGINFO *pip,int jtype,int stype,int ifd,int ofd)
{
	MFSWATCH	*wip = pip->watch ;
	LOCINFO		*lip = pip->lip ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_newjob: ent jtype=%u stype=%u\n",
		jtype,stype) ;
#endif

	if ((rs = locinfo_newserial(lip)) >= 0) {
	    const int	llen = LOGIDLEN ;
	    char	lbuf[LOGIDLEN+1] ;
	    if ((rs = mksublogid(lbuf,llen,pip->logid,rs)) >= 0) {
	        SREQDB	*slp = &wip->reqs ;
	        if ((rs = sreqdb_newjob(slp,lbuf,ifd,ofd)) >= 0) {
		    if ((rs = sreqdb_typeset(slp,rs,jtype,stype)) >= 0) {
			const int	re = (POLLIN | POLLPRI) ;
			rs = mfswatch_pollreg(pip,ifd,re) ;
		    }
	        } /* end if (sreqdb_newjob) */
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_newjob: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfswatch_newjob) */


/* private subroutines */


static int mfswatch_beginner(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs ;
	cchar		*td = pip->tmpdname ;
	if ((rs = sreqdb_start(&wip->reqs,td,0)) >= 0) {
	    if ((rs = mfswatch_svcbegin(pip)) >= 0) {
	        POLLER	*pmp = &wip->pm ;
	        if ((rs = poller_start(pmp)) >= 0) {
		    if ((rs = mfsadj_begin(pip)) >= 0) {
		        if ((rs = mfsadj_register(pip,pmp)) >= 0) {
			    rs = mfslisten_maint(pip,pmp) ;
		        }
		        if (rs < 0)
			    mfsadj_end(pip) ;
		    }
	            if (rs < 0)
		        poller_finish(pmp) ;
	        } /* end if (poller_start) */
		if (rs < 0) {
		    mfswatch_svcend(pip) ;
		}
	    } /* end if (mfswatch-svc) */
	    if (rs < 0)
		sreqdb_finish(&wip->reqs) ;
	} /* end if (sreqdb_start) */
	return rs ;
}
/* end subroutine (mfswatch_beginner) */


static int mfswatch_ender(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = mfsadj_end(pip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = poller_finish(&wip->pm) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mfswatch_svcend(pip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = sreqdb_finish(&wip->reqs) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_ender: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfswatch_ender) */


static int mfswatch_polljobs(PROGINFO *pip,int fd,int re)
{
	MFSWATCH	*wip = pip->watch ;
	SREQDB		*srp ;
	int		rs ;
	int		f = FALSE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_polljobs: ent fd=%d\n",fd) ;
#endif
	srp = &wip->reqs ;
	if ((rs = sreqdb_havefd(srp,fd)) >= 0) {
	    SREQ	*jep ;
	    if ((rs = sreqdb_get(srp,rs,&jep)) >= 0) {
		if ((rs = mfswatch_svcaccum(pip,jep,fd,re)) > 0) {
		    f = TRUE ;
#if	CF_DEBUG
		    if (DEBUGLEVEL(4))
		        debugprintf("mfswatch_polljobs: svcbuf=>%s<\n",
			jep->svcbuf) ;
#endif
		    rs = mfswatch_svcfind(pip,jep) ;
		} else if (rs == 0) {
		    re = (POLLIN | POLLPRI) ;
		    rs = mfswatch_pollreg(pip,fd,re) ;
		}
	    } /* end if (sreqdb_get) */
	} else if (rs == SR_NOTFOUND) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("mfswatch_polljobs: not-found\n") ;
#endif
	    rs = SR_OK ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_polljobs: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfswatch_polljobs) */


/* ARGSUSED */
static int mfswatch_svcaccum(PROGINFO *pip,SREQ *jep,int fd,int re)
{
	MFSWATCH	*wip = pip->watch ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		f = FALSE ;
	char		*lbuf ;
	if (wip == NULL) return SR_FAULT ;
	if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	    if ((rs = uc_peek(fd,lbuf,llen)) > 0) {
	        cchar	*tp ;
#if	CF_DEBUG
		if (DEBUGLEVEL(4))
	    	debugprintf("mfswatch_svcaccum: uc_peek() rs=%d\n",rs) ;
#endif
		if ((tp = strnchr(lbuf,rs,'\n')) != NULL) {
		    if ((rs = u_read(fd,lbuf,(tp-lbuf+1))) > 0) {
			f = TRUE ;
			rs = sreq_addsvc(jep,lbuf,(rs-1)) ;
		    } /* end if (u_read) */
		} else {
		    if ((rs = u_read(fd,lbuf,rs)) > 0) {
			rs = sreq_addsvc(jep,lbuf,rs) ;
		    }
		}
	    } /* end if (uc_peek) */
	    uc_free(lbuf) ;
	} /* end if (m-a-f) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcaccum: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfswatch_svcaccum) */


static int mfswatch_pollreg(PROGINFO *pip,int fd,int re)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs ;
	{
	    POLLER	*pmp = &wip->pm ;
	    POLLER_SPEC	ps ;
	    ps.fd = fd ;
	    ps.events = re ;
	    rs = poller_reg(pmp,&ps) ;
	}
	return rs ;
}
/* end subroutine (mfswatch_pollreg) */


static int mfswatch_svcbegin(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	LOCINFO		*lip ;
	int		rs = SR_OK ;
#if	CF_SVC
	lip = pip->lip ;
	if ((lip->svcfname != NULL) && (lip->svcfname[0] != '\0')) {
	    SVCFILE	*sfp = &wip->sdb ;
	    cchar	*svcfname = lip->svcfname ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("mfswatch_svcbegin: sfn=%s\n",svcfname) ;
#endif
	    if ((rs = svcfile_open(sfp,svcfname)) >= 0) {
		wip->open.sdb = TRUE ;
	    }
	} /* end if */
#endif /* CF_SVC */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcbegin: ret rs=%d open.sdb=%u\n",
		rs,wip->open.sdb) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_svcbegin) */


static int mfswatch_svcend(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcend: ent open.sdb=%u\n",
		wip->open.sdb) ;
#endif
	if (wip->open.sdb) {
	    SVCFILE	*sfp = &wip->sdb ;
	    wip->open.sdb = FALSE ;
	    rs1 = svcfile_close(sfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (is-open) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_svcend) */


static int mfswatch_svcfind(PROGINFO *pip,SREQ *jep)
{
	MFSWATCH	*wip = pip->watch ;
	vecstr		sa ; /* server arguments */
	int		rs ;
	int		rs1 ;
	if (wip == NULL) return SR_FAULT ;
	if ((rs = vecstr_start(&sa,1,0)) >= 0) {
	    if ((rs = vecstr_svcargs(&sa,jep->svcbuf)) >= 0) {
		if ((rs = sreq_setsvc(jep,rs)) >= 0) {
		    rs = mfswatch_svcfinder(pip,jep,&sa) ;
		} /* end if (sreq_setlong) */
	    } /* end if (vecstr_svcargs) */
	    rs1 = vecstr_finish(&sa) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcfind: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_svcfind) */


static int mfswatch_svcfinder(PROGINFO *pip,SREQ *jep,vecstr *sap)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs ;
	cchar		*svc ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcfinder: ent\n") ;
#endif
	if ((rs = sreq_getsvc(jep,&svc)) >= 0) {
	    const int	elen = MAX(SBUFLEN,rs) ;
	    char	*ebuf ;
	    if ((rs = uc_malloc((elen+1),&ebuf)) >= 0) {
	        SVCFILE		*slp = &wip->sdb ;
	        SVCFILE_ENT	e ;
	        if ((rs = svcfile_fetch(slp,svc,NULL,&e,ebuf,elen)) >= 0) {
    		    rs = mfswatch_svcproc(pip,jep,&e,sap) ;
	        } else if (isNotPresent(rs)) {
		    if ((rs = mfswatch_notfound(pip,jep)) >= 0) {
		        rs = mfswatch_jobretire(pip,jep) ;
		    }
	        }
	        uc_free(ebuf) ;
	    } /* end if (m-a-f) */
	} /* end if (sreq_getsvc) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcfinder: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_svcfinder) */


/* ARGSUSED */
static int mfswatch_svcproc(PROGINFO *pip,SREQ *jep,SVCFILE_ENT *sep,
		vecstr *sap)
{
	LOCINFO		*lip = pip->lip ;
	const int	f_long = jep->f_long ;
	int		rs ;
	int		rs1 ;
	if (sap == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcproc: ent\n") ;
#endif
	    if ((rs = locinfo_svcload(lip,sep->svc,f_long)) >= 0) {
		SVCENTSUB	ss ;
	        if ((rs = svcentsub_start(&ss,lip,sep)) >= 0) {
#if	CF_DEBUG
		    if (DEBUGLEVEL(4)) {
	    	    debugprintf("mfswatch_svcproc: svc=%s\n",sep->svc) ;
	    	    debugprintf("mfswatch_svcproc: p=%s\n",
			ss.var[svckey_p]) ;
	    	    debugprintf("mfswatch_svcproc: a=%s\n",
			ss.var[svckey_a]) ;
	    	    debugprintf("mfswatch_svcproc: so=%s\n",
			ss.var[svckey_so]) ;
		    }
#endif
    
    
	            rs1 = svcentsub_finish(&ss) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (svcentsub) */
	    } /* end if (locinfo_svcload) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcproc: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_svcproc) */


#ifdef	COMMENT
static int mfswatch_islong(PROGINFO *pip,vecstr *sap)
{
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;
	cchar		*ap ;
	for (i = 0 ; vecstr_get(sap,i,&ap) >= 0 ; i += 1) {
	    if ((ap != NULL) && (ap[0] == '/')) {
		    f = ((ap[1] == 'w') || (ap[1] == 'W')) ;
		    if (f) break ;
	    }
	} /* end for */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfswatch_islong) */
#endif /* COMMENT */


static int mfswatch_notfound(PROGINFO *pip,SREQ *jep)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		fd = jep->ofd ;
	int		sl = -1 ;
	int		len = 0 ;
	cchar		*sp = NULL ;
	if (wip == NULL) return SR_FAULT ;
	if (fd < 0) fd = jep->ifd ;
	switch (pip->progmode) {
	case progmode_tcpmuxd:
	    sp = "- service not found\n" ;
	    break ;
	} /* end switch */
	if (sp != NULL) {
	    if (sl < 0) sl = strlen(sp) ;
	    if ((rs = uc_writen(fd,sp,sl)) >= 0) {
	        len = rs ;
	    } else if (isBadSend(rs)) {
	        rs = SR_OK ;
	    }
	}
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mfswatch_notfound) */


static int mfswatch_jobretire(PROGINFO *pip,SREQ *jep)
{
	MFSWATCH	*wip = pip->watch ;
	SREQDB		*dbp ;
	int		rs ;
	dbp = &wip->reqs ;
	rs = sreqdb_delp(dbp,jep) ;
	return rs ;
}
/* end subroutine (mfswatch_jobretire) */


