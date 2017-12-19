/* mfs-watch */

/* watch (listen on) the specified service-access-points */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	1		/* switchable debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */
#define	CF_SHLIB	0		/* shared-object library loading */


/* revision history:

	= 2008-06-23, David A­D­ Morano
        I updated this subroutine to just poll for machine status and write the
        Machine Status (MS) file. This was a cheap excuse for not writing a
        whole new daemon program just to poll for machine status. I hope this
        works out! :-)

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code the MFSERVE program.

*/

/* Copyright © 2008,2017 David A­D­ Morano.  All rights reserved. */

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

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

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
#include	<estrings.h>
#include	<bfile.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<vecpstr.h>
#include	<osetstr.h>
#include	<sockaddress.h>
#include	<connection.h>
#include	<poller.h>
#include	<svcfile.h>
#include	<upt.h>
#include	<envhelp.h>
#include	<spawnproc.h>
#include	<filebuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
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
#include	"svckv.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#ifndef	SVCNAMELEN
#define	SVCNAMELEN	MAXNAMELEN
#endif

#ifndef	CMSGBUFLEN
#define	CMSGBUFLEN	256
#endif

#define	MFSWATCH	struct mfswatch
#define	MFSWATCH_FL	struct mfswatch_flags

#define	SVCPROCARGS	struct svcprocargs

#define	W_OPTIONS	(WNOHANG)

#define	IPCBUFLEN	MSGBUFLEN

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


/* typedefs */

typedef int (*svcprocer_t)(PROGINFO *pip,SREQ *jep) ;


/* external subroutines */

extern int	pathadd(char *,int,cchar *) ;
extern int	pathaddw(char *,int,cchar *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	bufprintf(const char *,int,...) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	dupup(int,int) ;
extern int	passfd(cchar *,int) ;
extern int	nlspeername(const char *,const char *,char *) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	acceptpass(int,struct strrecvfd *,int) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	xfile(IDS *,cchar *) ;
extern int	varsub_addvec(VARSUB *,VECSTR *) ;
extern int	vecstr_srvargs(vecstr *,cchar *) ;
extern int	vecpstr_addpath(vecpstr *,cchar *) ;
extern int	vecpstr_addcspath(vecpstr *) ;
extern int	osetstr_loadfile(osetstr *,int,cchar *) ;
extern int	hasnonwhite(cchar *,int) ;
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

extern cchar	*getourenv(cchar **,cchar *) ;
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
	uint		built:1 ;
	uint		svcfile:1 ;
	uint		eh:1 ;		/* environment-mangement helper */
	uint		bindirs:1 ;	/* path-executable */
	uint		libdirs:1 ;	/* path-library */
	uint		users:1 ;
} ;

struct mfswatch {
	MFSWATCH_FL	f, open ;
	SREQDB		reqs ;		/* service requests */
	POLLER		pm ;		/* Poll-Manager */
	MFSBUILT	built ;		/* builtin lookup database */
	SVCFILE		tabs ;		/* service lookup database */
	ENVHELP		eh ;		/* environment-management helper */
	vecpstr		bindirs ;	/* path-executable */
	vecpstr		libdirs ;	/* path-library */
	osetstr		users ;
	time_t		ti_mark ;
	time_t		ti_maint ;
	time_t		ti_config ;
	time_t		ti_users ;
	int		nprocs ;
	int		nthrs ;
	int		nbuilts ;
	int		pollto ;	/* poll interval in milliseconds */
} ;

struct svcprocargs {
	PROGINFO	*pip ;
	SREQ		*jep ;
	svcprocer_t	w ;
} ;


/* forward references */

static int	mfswatch_beginner(PROGINFO *) ;
static int	mfswatch_ender(PROGINFO *) ;
static int	mfswatch_envbegin(PROGINFO *) ;
static int	mfswatch_envend(PROGINFO *) ;
static int	mfswatch_svcaccum(PROGINFO *,SREQ *,int,int) ;
static int	mfswatch_uptimer(PROGINFO *) ;
static int	mfswatch_configmaint(PROGINFO *) ;
static int	mfswatch_poll(PROGINFO *,POLLER_SPEC *) ;
static int	mfswatch_pollreg(PROGINFO *,int,int) ;
static int	mfswatch_polljobs(PROGINFO *,int,int) ;

static int	mfswatch_pathbegin(PROGINFO *) ;
static int	mfswatch_pathend(PROGINFO *) ;
static int	mfswatch_pathload(PROGINFO *,vecpstr *,cchar *) ;
static int	mfswatch_pathfindbin(PROGINFO *,char *,cchar *,int) ;
static int	mfswatch_pathfindbinpr(PROGINFO *,char *,cchar *,int) ;
static int	mfswatch_pathfindbinlist(PROGINFO *,char *,cchar *,int) ;

static int	mfswatch_svcsbegin(PROGINFO *) ;
static int	mfswatch_svcsend(PROGINFO *) ;
static int	mfswatch_svcsmaint(PROGINFO *) ;

static int	mfswatch_tabsbegin(PROGINFO *) ;
static int	mfswatch_tabsend(PROGINFO *) ;
static int	mfswatch_tabsmaint(PROGINFO *) ;

static int	mfswatch_usersbegin(PROGINFO *) ;
static int	mfswatch_usersend(PROGINFO *) ;
static int	mfswatch_usersload(PROGINFO *,osetstr *) ;
static int	mfswatch_usersmaint(PROGINFO *) ;
static int	mfswatch_usershave(PROGINFO *,cchar *) ;
static int	mfswatch_usershandle(PROGINFO *,SREQ *,cchar **) ;
static int	mfswatch_usershandler(PROGINFO *,SREQ *) ;
static int	mfswatch_usersproj(PROGINFO *,int,char *,int,cchar *) ;
static int	mfswatch_usersplan(PROGINFO *,int,char *,int,cchar *) ;
static int	mfswatch_usersfile(PROGINFO *,int,char *,int,cchar *) ;

static int	mfswatch_builtbegin(PROGINFO *) ;
static int	mfswatch_builtend(PROGINFO *) ;
static int	mfswatch_builtmaint(PROGINFO *) ;

static int mfswatch_svcfind(PROGINFO *,SREQ *) ;
static int mfswatch_svcfinder(PROGINFO *,SREQ *) ;
static int mfswatch_svcproc(PROGINFO *,SREQ *,SVCFILE_ENT *,cchar **) ;

static int mfswatch_svcprocer(PROGINFO *,SREQ *,svcprocer_t) ;

static int mfswatch_svcprocfile(PROGINFO *,SREQ *,SVCENT *) ;
static int mfswatch_svcprocpass(PROGINFO *,SREQ *,SVCENT *) ;
static int mfswatch_svcprocprog(PROGINFO *,SREQ *,SVCENT *) ;
static int mfswatch_svcprocproger(PROGINFO *,SREQ *,cchar *,vecstr *) ;
static int mfswatch_progspawn(PROGINFO *,SREQ *,cchar *,vecstr *) ;

#if	CF_SHLIB
static int mfswatch_tabsprocshlib(PROGINFO *,SREQ *,SVCENT *) ;
#endif

static int mfswatch_builthave(PROGINFO *,cchar *) ;
static int mfswatch_builthandle(PROGINFO *,SREQ *,cchar **) ;

static int mfswatch_svchelp(PROGINFO *,SREQ *) ;
static int mfswatch_svchelper(PROGINFO *,SREQ *) ;
static int mfswatch_loadsvcs(PROGINFO *,SREQ *) ;

static int mfswatch_svcprocfiler(PROGINFO *,SREQ *) ;

static int mfswatch_svcretstat(PROGINFO *,SREQ *,int) ;
static int mfswatch_jobretire(PROGINFO *,SREQ *) ;
static int mfswatch_checkthrs(PROGINFO *) ;
static int mfswatch_checkprogs(PROGINFO *) ;
static int mfswatch_checkbuilts(PROGINFO *) ;
static int mfswatch_checkbuilt(PROGINFO *,SREQ *) ;
static int mfswatch_logprogres(PROGINFO *,int,cchar *,pid_t,int) ;
static int mfswatch_logprogchild(PROGINFO *,SREQ *) ;

static int mfswatch_thrdone(PROGINFO *,SREQ *) ;
static int mfswatch_logconn(PROGINFO *,int,int,int,cchar *) ;
static int mfswatch_logsvc(PROGINFO *,SREQ *) ;

#if	defined(COMMENT)
static int	mfswatch_islong(PROGINFO *,vecstr *) ;
#endif

static int	svcprocers(SVCPROCARGS *) ;


/* local variables */

static cchar	*envbads[] = {
	"_",
	"_A0",
	"_EF",
	"A__z",
	"RANDOM",
	"SECONDS",
	NULL
} ;

static cchar	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;


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
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_service: ent\n") ;
#endif

	if ((rs = mfswatch_uptimer(pip)) >= 0) {
	    POLLER	*pmp = &wip->pm ;
	    POLLER_SPEC	ps ;

	    if ((rs = poller_wait(pmp,&ps,wip->pollto)) > 0) {
		LOCINFO		*lip = pip->lip ;
	        pip->daytime = time(NULL) ;
	        if ((rs = mfswatch_poll(pip,&ps)) >= 0) {
	            c += rs ;
	            while ((rs = poller_get(pmp,&ps)) > 0) {
	                rs = mfswatch_poll(pip,&ps) ;
	                c += rs ;
	                if (rs < 0) break ;
	            } /* end while */
	        } /* end if */
		if (rs >= 0) {
		    rs = locinfo_newreq(lip,c) ;
		}
	    } else if (rs == 0) {
	        pip->daytime = time(NULL) ;
	    } else if (rs == SR_INTR) {
	        pip->daytime = time(NULL) ;
	        rs = SR_OK ;
	    } /* end if */

	    if (rs >= 0) {
	        if ((rs = mfswatch_checkthrs(pip)) >= 0) {
	            if ((rs = mfswatch_checkprogs(pip)) >= 0) {
			rs = mfswatch_checkbuilts(pip) ;
		    }
	        }
	    } /* end if (ok) */

	    if (rs >= 0) {
	        const int	to = TO_MAINT ;
	        if ((pip->daytime - wip->ti_maint) >= to) {
	            wip->ti_maint = pip->daytime ;
	            if ((rs = mfslisten_maint(pip,pmp)) >= 0) {
	                rs = mfswatch_svcsmaint(pip) ;
	            }
	        }
	    }

	    if (rs >= 0) {
	        rs = mfswatch_configmaint(pip) ;
	    }

	} /* end if (mfswatch_uptimer) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_service: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfswatch_service) */


/* local subroutines */


/* update our poll time-out value */
static int mfswatch_uptimer(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	const int	max = (pip->intpoll * POLLINTMULT) ;
	int		njobs ;

	njobs = (wip->nprocs + wip->nthrs + wip->nbuilts) ;

	if (njobs <= 0) {
	    if (wip->pollto < 100) {
	        wip->pollto = 100 ;
	    }
	    wip->pollto += 100 ;
	} else {
	    wip->pollto += 10 ;
	}

	if (wip->pollto < 10) {
	    wip->pollto = 10 ;
	} else if (wip->pollto > max) {
	    wip->pollto = max ;
	}

#if	CF_DEBUG
	{
	    if (DEBUGLEVEL(5))
	        debugprintf("mfswatch_uptimer: pollto=%u\n",wip->pollto) ;
	}
#endif

	return SR_OK ;
}
/* end subroutine (mfswatch_uptimer) */


static int mfswatch_configmaint(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	if (pip->open.config) {
	    const int	to = TO_CONFIG ;
	    if ((pip->daytime - wip->ti_config) >= to) {
	        CONFIG	*cfp = pip->config ;
	        wip->ti_config = pip->daytime ;
	        if ((rs = config_check(cfp)) > 0) {
	            cchar	*fmt = "%s configuration data-base changed\n" ;
	            char	tbuf[TIMEBUFLEN+1] ;
	            timestr_logz(pip->daytime,tbuf) ;
	            rs = logprintf(pip,fmt,tbuf) ;
	        }
	    }
	}
	return rs ;
}
/* end subroutine (mfswatch_configmaint) */


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
	    debugprintf("mfswatch_poll: ent fd=%u re=(%s)\n",fd,pbuf) ;
	}
#endif /* CF_DEBUG */

	pmp = &wip->pm ;
	if ((rs = mfsadj_poll(pip,pmp,fd,re)) > 0) {
	    f = TRUE ;
	} else if (rs == 0) {
	    if ((rs = mfslisten_poll(pip,pmp,fd,re)) > 0) {
	        f = TRUE ;
	    } else if (rs == 0) {
	        if ((rs = mfswatch_polljobs(pip,fd,re)) == 0) {
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
	    const int	jsn = rs ;
	    const int	llen = LOGIDLEN ;
	    char	lbuf[LOGIDLEN+1] ;
	    if ((rs = mksublogid(lbuf,llen,pip->logid,jsn)) >= 0) {
	        SREQDB	*slp = &wip->reqs ;
	        if ((rs = sreqdb_newjob(slp,jsn,lbuf,ifd,ofd)) >= 0) {
	            if ((rs = sreqdb_typeset(slp,rs,jtype,stype)) >= 0) {
	                const int	re = (POLLIN | POLLPRI) ;
	                if ((rs = mfswatch_pollreg(pip,ifd,re)) >= 0) {
	                    rs = mfswatch_logconn(pip,jsn,jtype,stype,lbuf) ;
	                }
	            }
	        } /* end if (sreqdb_newjob) */
	    }
	} /* end if (locinfo_newserial) */

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
	if ((rs = mfswatch_envbegin(pip)) >= 0) {
	    if ((rs = mfswatch_pathbegin(pip)) >= 0) {
	        if ((rs = sreqdb_start(&wip->reqs,td,0)) >= 0) {
	            if ((rs = mfswatch_svcsbegin(pip)) >= 0) {
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
	                    mfswatch_svcsend(pip) ;
	                }
	            } /* end if (mfswatch-svc) */
	            if (rs < 0)
	                sreqdb_finish(&wip->reqs) ;
	        } /* end if (sreqdb_start) */
	        if (rs < 0)
	            mfswatch_pathend(pip) ;
	    } /* end if (mfswatch_pathbegin) */
	    if (rs < 0)
	        mfswatch_envend(pip) ;
	} /* end if (mfswatch_envbegin) */
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

	rs1 = mfswatch_svcsend(pip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_ender: mid3 rs=%d\n",rs) ;
#endif

	rs1 = sreqdb_finish(&wip->reqs) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_ender: mid4 rs=%d\n",rs) ;
#endif

	rs1 = mfswatch_pathend(pip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_ender: mid5 rs=%d\n",rs) ;
#endif

	rs1 = mfswatch_envend(pip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_ender: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfswatch_ender) */


static int mfswatch_envbegin(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	ENVHELP		*ehp ;
	int		rs ;
	ehp = &wip->eh ;
	if ((rs = envhelp_start(ehp,envbads,pip->envv)) >= 0) {
	    wip->open.eh = TRUE ;
	}
	return rs ;
}
/* end subroutine (mfswatch_envbegin) */


static int mfswatch_envend(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (wip->open.eh) {
	    ENVHELP	*ehp = &wip->eh ;
	    wip->open.eh = FALSE ;
	    rs1 = envhelp_finish(ehp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (mfswatch_envend) */


static int mfswatch_pathbegin(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs ;
	cchar		*var = VARBINPATH ;
	if ((rs = mfswatch_pathload(pip,&wip->bindirs,var)) >= 0) {
	    var = VARLIBPATH ;
	    wip->open.bindirs = TRUE ;
	    if ((rs = mfswatch_pathload(pip,&wip->libdirs,var)) >= 0) {
	        wip->open.libdirs = TRUE ;
	    }
	    if (rs < 0) {
	        wip->open.bindirs = FALSE ;
	        vecpstr_finish(&wip->bindirs) ;
	    }
	}
	return rs ;
}
/* end subroutine (mfswatch_pathbegin) */


static int mfswatch_pathend(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (wip->open.bindirs) {
	    wip->open.bindirs = FALSE ;
	    rs1 = vecpstr_finish(&wip->bindirs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (wip->open.libdirs) {
	    wip->open.libdirs = FALSE ;
	    rs1 = vecpstr_finish(&wip->libdirs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (mfswatch_pathend) */


static int mfswatch_pathload(PROGINFO *pip,vecpstr *plp,cchar *var)
{
	const int	n = 40 ;
	const int	cs = 1024 ;
	int		rs ;
	if ((rs = vecpstr_start(plp,n,cs,0)) >= 0) {
	    cchar	*cp = getourenv(pip->envv,var) ;
	    if (hasnonwhite(cp,-1)) {
	        rs = vecpstr_addpath(plp,cp) ;
	    }
	    if (rs >= 0) {
	        rs = vecpstr_addcspath(plp) ;
	    }
	    if (rs < 0)
	        vecpstr_finish(plp) ;
	}
	return rs ;
}
/* end subroutine (mfswatch_pathload) */


static int mfswatch_pathfindbin(PROGINFO *pip,char *rbuf,cchar *np,int nl)
{
	int		rs ;
	int		pl = 0 ;
	if (nl < 0) nl = strlen(np) ;
	if (strnchr(np,nl,'/') == NULL) {
	    if ((rs = mfswatch_pathfindbinpr(pip,rbuf,np,nl)) == 0) {
	        rs = mfswatch_pathfindbinlist(pip,rbuf,np,nl) ;
	        pl = rs ;
	    } else {
	        pl = rs ;
	    }
	} else {
	    if (np[0] != '/') {
	        rs = mkpath2w(rbuf,pip->pr,np,nl) ;
	    } else {
	        rs = mkpath1w(rbuf,np,nl) ;
	    }
	    if (rs >= 0) {
	        IDS	*idp = &pip->id ;
	        pl = rs ;
	        rs = xfile(idp,rbuf) ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("mfswatch_pathfindbin: ret rs=%d pl=%u\n",rs,pl) ;
	    debugprintf("mfswatch_pathfindbin: rbuf=%s\n",rbuf) ;
	}
#endif
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mfswatch_pathfindbin) */


static int mfswatch_pathfindbinpr(PROGINFO *pip,char *rbuf,cchar *np,int nl)
{
	IDS		*idp = &pip->id ;
	int		rs ;
	int		pl = 0 ;
	cchar		*pr = pip->pr ;
	if ((rs = mkpath1(rbuf,pr)) >= 0) {
	    const int	rlen = rs ;
	    int		i ;
	    for (i = 0 ; prbins[i] != NULL ; i += 1) {
	        cchar	*bin = prbins[i] ;
	        if ((rs = pathadd(rbuf,rlen,bin)) >= 0) {
	            if ((rs = pathaddw(rbuf,rs,np,nl)) >= 0) {
	                pl = rs ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("mfswatch_pathfindbinpr: "
	                        "pl=%d rbuf=%s\n",pl, rbuf) ;
#endif
	                if (((rs = xfile(idp,rbuf)) < 0) && isNotAccess(rs)) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("mfswatch_pathfindbinpr: "
	                            "xfile() rs=%d\n",rs) ;
#endif
	                    rs = SR_OK ;
	                    pl = 0 ;
	                }
	            }
	        }
	        if (pl > 0) break ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (mkpath) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("mfswatch_pathfindbinpr: ret rs=%d pl=%u\n",rs,pl) ;
	    debugprintf("mfswatch_pathfindbinpr: rbuf=%s\n",rbuf) ;
	}
#endif
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mfswatch_pathfindbinpr) */


static int mfswatch_pathfindbinlist(PROGINFO *pip,char *rbuf,cchar *np,int nl)
{
	MFSWATCH	*wip = pip->watch ;
	IDS		*idp = &pip->id ;
	vecpstr		*plp ;
	int		rs = SR_OK ;
	int		pl = 0 ;
	int		i ;
	cchar		*pp ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_pathfindbinlist: ent\n") ;
#endif
	plp = &wip->bindirs ;
	for (i = 0 ; vecpstr_get(plp,i,&pp) >= 0 ; i += 1) {
	    if (pp != NULL) {
	        if ((rs = mkpath2w(rbuf,pp,np,nl)) >= 0) {
	            pl = rs ;
	            if (((rs = xfile(idp,rbuf)) < 0) && isNotPresent(rs)) {
	                rs = SR_OK ;
	                pl = 0 ;
	            }
	        }
	    }
	    if (pl > 0) break ;
	    if (rs < 0) break ;
	} /* end for */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_pathfindbinlist: ret rs=%d pl=%u\n",rs,pl) ;
#endif
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mfswatch_pathfindbinlist) */


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
	        if ((rs = sreq_getstate(jep)) == sreqstate_acquire) {
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
	        } /* end if (in acquire state */
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


/* register this FD w/ the poller object */
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


/* ARGSUSED */
static int mfswatch_svcaccum(PROGINFO *pip,SREQ *jep,int fd,int re)
{
	MFSWATCH	*wip = pip->watch ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		f = FALSE ;
	char		*lbuf ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcaccum: ent\n") ;
#endif
	if (wip == NULL) return SR_FAULT ;
	if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	    if ((rs = uc_peek(fd,lbuf,llen)) > 0) {
	        cchar	*tp ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("mfswatch_svcaccum: uc_peek() rs=%d\n",rs) ;
#endif
	        if ((tp = strnchr(lbuf,rs,'\n')) != NULL) {
		    const int	ll = (tp-lbuf+1) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("mfswatch_svcaccum: l=>%t<\n",
			lbuf,strlinelen(lbuf,ll,50)) ;
#endif
	            if ((rs = u_read(fd,lbuf,ll)) > 0) {
	                if ((rs = sreq_svcaccum(jep,lbuf,(rs-1))) >= 0) {
	                    f = TRUE ;
			    if ((rs = sreq_svcmunge(jep)) >= 0) {
	                        const int	js = sreqstate_svc ;
	                        rs = sreq_setstate(jep,js) ;
			    }
	                }
	            } /* end if (u_read) */
	        } else {
	            if ((rs = u_read(fd,lbuf,rs)) > 0) {
	                rs = sreq_svcaccum(jep,lbuf,rs) ;
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


static int mfswatch_svcfind(PROGINFO *pip,SREQ *jep)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs ;
	if (wip == NULL) return SR_FAULT ;
	if ((rs = mfswatch_logsvc(pip,jep)) >= 0) {
	                rs = mfswatch_svcfinder(pip,jep) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcfind: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_svcfind) */


static int mfswatch_svcfinder(PROGINFO *pip,SREQ *jep)
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
	        SVCFILE		*slp = &wip->tabs ;
	        SVCFILE_ENT	e ;
	        cchar		**sav ;
	        if ((rs = sreq_getav(jep,&sav)) >= 0) {
	            const int	rsn = SR_NOTFOUND ;
		    const int	t = TRUE ;
	            if ((rs = svcfile_fetch(slp,svc,NULL,&e,ebuf,elen)) >= 0) {
	                if ((rs = mfswatch_svcretstat(pip,jep,t)) >= 0) {
	                    rs = mfswatch_svcproc(pip,jep,&e,sav) ;
	                }
	            } else if (rs == rsn) {
	                if ((rs = mfswatch_usershave(pip,svc)) > 0) {
	                    if ((rs = mfswatch_svcretstat(pip,jep,t)) >= 0) {
	                        rs = mfswatch_usershandle(pip,jep,sav) ;
	                    }
	                } else if ((rs = mfswatch_builthave(pip,svc)) > 0) {
	                    if ((rs = mfswatch_svcretstat(pip,jep,t)) >= 0) {
	                        rs = mfswatch_builthandle(pip,jep,sav) ;
	                    }
	                } else if (strcmp(svc,"help") == 0) {
	                    if ((rs = mfswatch_svcretstat(pip,jep,t)) >= 0) {
	                        rs = mfswatch_svchelp(pip,jep) ;
	                    }
	                } else if (rs == 0) {
	                    const int	f = FALSE ;
	                    if ((rs = mfswatch_svcretstat(pip,jep,f)) >= 0) {
	                        rs = mfswatch_jobretire(pip,jep) ;
	                    }
	                }
	            } /* end if */
	        } /* end if (vecstr_getvec) */
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


static int mfswatch_svcsbegin(PROGINFO *pip)
{
	int		rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcsbegin: ent\n") ;
#endif
	if ((rs = mfswatch_builtbegin(pip)) >= 0) {
	    if ((rs = mfswatch_usersbegin(pip)) >= 0) {
	        rs = mfswatch_tabsbegin(pip) ;
	        if (rs < 0)
	            mfswatch_usersend(pip) ;
	    }
	    if (rs < 0)
	        mfswatch_builtend(pip) ;
	}
	return rs ;
}
/* end subroutine (mfswatch_svcsbegin) */


static int mfswatch_svcsend(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcsend: ent\n") ;
#endif

	rs1 = mfswatch_tabsend(pip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mfswatch_usersend(pip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mfswatch_builtend(pip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcsend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfswatch_svcsend) */


static int mfswatch_svcsmaint(PROGINFO *pip)
{
	int		rs ;
	if ((rs = mfswatch_builtmaint(pip)) >= 0) {
	    if ((rs = mfswatch_usersmaint(pip)) >= 0) {
	        rs = mfswatch_tabsmaint(pip) ;
	    }
	}
	return rs ;
}
/* end subroutine (mfswatch_svcsmaint) */


static int mfswatch_tabsbegin(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	LOCINFO		*lip ;
	int		rs = SR_OK ;
	lip = pip->lip ;
	if ((lip->svcfname != NULL) && (lip->svcfname[0] != '\0')) {
	    SVCFILE	*sfp = &wip->tabs ;
	    cchar	*svcfname = lip->svcfname ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("mfswatch_tabsbegin: sfn=%s\n",svcfname) ;
#endif
	    if ((rs = svcfile_open(sfp,svcfname)) >= 0) {
	        wip->open.svcfile = TRUE ;
	    }
	} /* end if */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_tabsbegin: ret rs=%d open.svcfile=%u\n",
	        rs,wip->open.svcfile) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_tabsbegin) */


static int mfswatch_tabsend(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_tabsend: ent open.svcfile=%u\n",
	        wip->open.svcfile) ;
#endif
	if (wip->open.svcfile) {
	    SVCFILE	*sfp = &wip->tabs ;
	    wip->open.svcfile = FALSE ;
	    rs1 = svcfile_close(sfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (is-open) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_tabsend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_tabsend) */


static int mfswatch_tabsmaint(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	if (wip->open.svcfile) {
	    SVCFILE	*sfp = &wip->tabs ;
	    if ((rs = svcfile_check(sfp,pip->daytime)) > 0) {
	        cchar	*fmt = "%s service data-base changed\n" ;
	        char	tbuf[TIMEBUFLEN+1] ;
	        timestr_logz(pip->daytime,tbuf) ;
	        rs = logprintf(pip,fmt,tbuf) ;
	    }
	}
	return rs ;
}
/* end subroutine (mfswatch_tabsmaint) */


static int mfswatch_usersbegin(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	if (lip->f.users) {
	    if (! wip->open.users) {
		OSETSTR		*ulp = &wip->users ;
		const int	n = DEFNUSERS ;
		if ((rs = osetstr_start(ulp,n)) >= 0) {
		    wip->open.users = TRUE ;
		    rs = mfswatch_usersload(pip,ulp) ;
		    if (rs < 0) {
		        wip->open.users = FALSE ;
			osetstr_finish(ulp) ;
		    }
		} /* end if (osetstr_start) */
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_usersbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_usersbegin) */


static int mfswatch_usersend(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (wip->open.users) {
	    OSETSTR	*ulp = &wip->users ;
	    wip->open.users = FALSE ;
	    rs1 = osetstr_finish(ulp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (mfswatch_usersend) */


static int mfswatch_usersload(PROGINFO *pip,osetstr *ulp)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	if (wip->open.users) {
	    OSETSTR	*ulp = &wip->users ;
	    cchar	*ufn = "/sys/users" ;
	    if ((rs = osetstr_loadfile(ulp,0,ufn)) >= 0) {
		wip->ti_users = time(NULL) ;
	    }
	}
	return rs ;
}
/* end subrlutine (mfswatch_usersload) */


static int mfswatch_usersmaint(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	if (wip->open.users) {
	    const time_t	dt = pip->daytime ;
	    const int		to = TO_USERSMAINT ;
	    if ((dt - wip->ti_users) >= to) {
	        OSETSTR	*ulp = &wip->users ;
		if ((rs = osetstr_delall(ulp)) >= 0) {
		    rs = mfswatch_usersload(pip,ulp) ;
		}
	    }
	}
	return rs ;
}
/* end subrlutine (mfswatch_usersmaint) */


static int mfswatch_usershave(PROGINFO *pip,cchar *sp)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	if (wip->open.users) {
	    OSETSTR	*ulp = &wip->users ;
	    rs = osetstr_already(ulp,sp,-1) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_usershave: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subrlutine (mfswatch_usershave) */


/* ARGSUSED */
static int mfswatch_usershandle(PROGINFO *pip,SREQ *jep,cchar **sav)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		f = FALSE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_usershandle: ent\n") ;
#endif
	if (wip->open.users) {
	    svcprocer_t		w = (svcprocer_t) mfswatch_usershandler ;
	    if ((rs = mfswatch_svcprocer(pip,jep,w)) >= 0) {
	        f = TRUE ;
	    }
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfswatch_usershandle) */


/* this is an independent thread */
static int mfswatch_usershandler(PROGINFO *pip,SREQ *jep)
{
	const int	hlen = MAXPATHLEN ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	char		hbuf[MAXPATHLEN+1] ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_usershandler: ent\n") ;
#endif
	if ((rs = getuserhome(hbuf,hlen,jep->svc)) > 0) {
	    const int	llen = LINEBUFLEN ;
	    char	*lbuf ;
	    if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	    if ((rs = sreq_getstdout(jep)) >= 0) {
		const int	ofd = rs ;
		if ((rs = mfswatch_usersproj(pip,ofd,lbuf,llen,hbuf)) >= 0) {
		    wlen += rs ;
		    rs = mfswatch_usersplan(pip,ofd,lbuf,llen,hbuf) ;
		    wlen += rs ;
		}
	    } /* end if (sreq_getstdout) */
	        rs1 = uc_free(lbuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a-f) */
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_usershandler: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfswatch_usershandler) */


static int mfswatch_usersproj(PROGINFO *pip,int ofd,char *lbuf,int llen,
		cchar *hbuf)
{
	int		rs ;
	int		wlen = 0 ;
	cchar		*fn = PROJFNAME ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mkpath2(tbuf,hbuf,fn)) >= 0) {
	    rs = mfswatch_usersfile(pip,ofd,lbuf,llen,tbuf) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfswatch_usersproj) */


static int mfswatch_usersplan(PROGINFO *pip,int ofd,char *lbuf,int llen,
		cchar *hbuf)
{
	int		rs ;
	int		wlen = 0 ;
	cchar		*fn = PLANFNAME ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mkpath2(tbuf,hbuf,fn)) >= 0) {
	    rs = mfswatch_usersfile(pip,ofd,lbuf,llen,tbuf) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfswatch_usersplan) */


static int mfswatch_usersfile(PROGINFO *pip,int ofd,char *lbuf,int llen,
		cchar *fn)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const mode_t	om = 0666 ;
	const int	of = O_RDONLY ;
	const int	to = TO_OPEN ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_usersfile: ent\n") ;
#endif
	if ((rs = uc_opene(fn,of,om,to)) >= 0) {
	    const int	fd = rs ;
	    const int	ro = 0 ;
	    while ((rs = uc_reade(fd,lbuf,llen,to,ro)) > 0) {
#if	CF_DEBUG
		if (DEBUGLEVEL(4))
		    debugprintf("mfswatch_usersfile: l=>%t<\n",
		        lbuf,strlinelen(lbuf,rs,50)) ;
#endif
		rs = uc_writen(ofd,lbuf,rs) ;
		wlen += rs ;
		if (rs < 0) break ;
	    } /* end while */
	    rs1 = u_close(fd) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (rs == SR_TIMEDOUT) {
	    rs = SR_OK ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_usersfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfswatch_usersfile) */


static int mfswatch_builtbegin(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		c = 0 ;
	if ((rs = locinfo_builtdname(lip)) >= 0) {
	    if (lip->builtdname != NULL) {
	        MFSBUILT	*blp = &wip->built ;
	        cchar		*bdn = lip->builtdname ;
	        if ((rs = mfsbuilt_start(blp,bdn)) >= 0) {
		    c += rs ;
	            wip->open.built = TRUE ;
	        }
	    }
	} /* end if (locinfo_builtdname) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_builtbegin: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfswatch_builtbegin) */


static int mfswatch_builtend(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_builtend: ent\n") ;
#endif
	if (wip->open.built) {
	    SREQDB	*slp = &wip->reqs ;
	    if ((rs = sreqdb_builtrelease(slp)) >= 0) {
	        MFSBUILT	*blp = &wip->built ;
	        wip->open.built = FALSE ;
	        rs1 = mfsbuilt_finish(blp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end if (is-open) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_builtend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_builtend) */


static int mfswatch_builtmaint(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	if (wip->open.built) {
	    MFSBUILT	*blp = &wip->built ;
	    if ((rs = mfsbuilt_check(blp,pip->daytime)) > 0) {
	        cchar	*fmt = "%s built-in modules changed\n" ;
	        char	tbuf[TIMEBUFLEN+1] ;
	        timestr_logz(pip->daytime,tbuf) ;
	        rs = logprintf(pip,fmt,tbuf) ;
	    }
	}
	return rs ;
}
/* end subroutine (mfswatch_builtmaint) */


/* ARGSUSED */
static int mfswatch_svcproc(PROGINFO *pip,SREQ *jep,SVCFILE_ENT *sep,
		cchar **sav)
{
	LOCINFO		*lip = pip->lip ;
	const int	f_long = jep->f.longopt ;
	int		rs ;
	cchar		*subsvc = jep->subsvc ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcproc: ent\n") ;
#endif
	if ((rs = locinfo_cooksvc(lip,sep->svc,subsvc,sav,f_long)) >= 0) {
	    if ((rs = sreq_svcentbegin(jep,lip,sep)) >= 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("mfswatch_svcproc: svc=%s\n",sep->svc) ;
	            debugprintf("mfswatch_svcproc: file=%s\n",
	                jep->ss.var[svckey_file]) ;
	            debugprintf("mfswatch_svcproc: p=%s\n",
	                jep->ss.var[svckey_p]) ;
	            debugprintf("mfswatch_svcproc: a=»%s«\n",
	                jep->ss.var[svckey_a]) ;
	            debugprintf("mfswatch_svcproc: so=%s\n",
	                jep->ss.var[svckey_so]) ;
	        }
#endif /* CF_DEBUG */
	        if ((rs = mfswatch_svcprocfile(pip,jep,sep)) == 0) {
	            if ((rs = mfswatch_svcprocpass(pip,jep,sep)) == 0) {
	                if ((rs = mfswatch_svcprocprog(pip,jep,sep)) == 0) {
	                    cchar	*fmt = "no service handler" ;
	                    logssprintf(pip,jep->logid,fmt) ;
	                    rs = mfswatch_jobretire(pip,jep) ;
	                }
	            }
	        }
	        if (rs < 0)
	            sreq_svcentend(jep) ;
	    } /* end if (sreq_svcentbegin) */
	} /* end if (locinfo_cooksvc) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcproc: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_svcproc) */


static int mfswatch_svcprocfile(PROGINFO *pip,SREQ *jep,SVCENT *sep)
{
	const int	n = sep->nkeys ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	cchar		*(*kv)[2] = sep->keyvals ;
	cchar		*vp ;
	if (pip == NULL) return SR_FAULT ;
	if (jep == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcprocfile: ent\n") ;
#endif
	if ((rs = svckv_isfile(kv,n,&vp)) > 0) {
	    svcprocer_t		w = (svcprocer_t) mfswatch_svcprocfiler ;
	    if ((rs = mfswatch_svcprocer(pip,jep,w)) >= 0) {
	        f = TRUE ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcprocfile: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfswatch_svcprocfile) */


/* this runs as an independent thread */
static int mfswatch_svcprocfiler(PROGINFO *pip,SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*fn = jep->ss.var[svckey_file] ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcprocfiler: ent fn=%s\n",fn) ;
#endif
	if ((fn != NULL) && (fn[0] != '\0')) {
	    const int	llen = LINEBUFLEN ;
	    char	*lbuf ;
	    if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	        if ((rs = sreq_ofd(jep)) >= 0) {
	            const mode_t	om = 0666 ;
	            const int		ofd = rs ;
	            const int		of = O_RDONLY ;
	            if ((rs = uc_open(fn,of,om)) >= 0) {
	                const int	fd = rs ;
	                const int	to = TO_READ ;
	                const int	ro = FM_TIMED ;
	                while ((rs = uc_reade(fd,lbuf,llen,to,ro)) > 0) {
	                    rs = uc_writen(ofd,lbuf,rs) ;
	                    wlen += rs ;
	                    if (rs < 0) break ;
	                } /* end while */
	                rs1 = u_close(fd) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (file) */
	        } /* end if (sreq_ofd) */
	        rs1 = uc_free(lbuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a-f) */
	} /* end if (non-null) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcprocfiler: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfswatch_svcprocfiler) */


static int mfswatch_svcprocpass(PROGINFO *pip,SREQ *jep,SVCENT *sep)
{
	const int	n = sep->nkeys ;
	int		rs ;
	int		f = FALSE ;
	cchar		*(*kv)[2] = sep->keyvals ;
	cchar		*vp ;
	if (pip == NULL) return SR_FAULT ;
	if (jep == NULL) return SR_FAULT ;
	if (sep == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcprocpass: ent\n") ;
#endif
	if ((rs = svckv_ispass(kv,n,&vp)) > 0) {
	    cchar	*lid = jep->logid ;
	    cchar	*fmt ;
	    cchar	*fn = jep->ss.var[svckey_pass] ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("mfswatch_svcprocpass: fn=%s\n",fn) ;
#endif
	    if (fn[0] != '\0') {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("mfswatch_svcprocpass: passfd() \n") ;
#endif
	        if ((rs = passfd(fn,jep->ifd)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("mfswatch_svcprocpass: passfd() rs=%d\n",
				rs) ;
#endif
	            f = TRUE ;
	            fmt = "pass=%s" ;
	            logssprintf(pip,lid,fmt,fn) ;
	            rs = mfswatch_jobretire(pip,jep) ;
	        } else if (isNotPresent(rs)) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("mfswatch_svcprocpass: pass-failed rs=%d\n",
	                    rs) ;
#endif
	            fmt = "pass=%s failed (%d)" ;
	            logssprintf(pip,lid,fmt,fn,rs) ;
	            rs = mfswatch_jobretire(pip,jep) ;
	        } /* end if (file) */
	    } /* end if (non-empty) */
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcprocpass: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ?  f : rs ;
}
/* end subroutine (mfswatch_svcprocpass) */


#if	CF_SHLIB
static int mfswatch_tabsprocshlib(PROGINFO *pip,SREQ *jep,SVCENT *sep)
{
	int		rs = SR_OK ;
	int		vl ;
	int		f = FALSE ;
	cchar		*vp ;
	if ((vl = svcent_islib(sep,&vp)) > 0) {
	    f = TRUE ;

	}
	return (rs >= 0) ?  f : rs ;
}
/* end subroutine (mfswatch_tabsprocshlib) */
#endif /* CF_SHLIB */


static int mfswatch_svcprocprog(PROGINFO *pip,SREQ *jep,SVCENT *sep)
{
	const int	n = sep->nkeys ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	cchar		*(*kv)[2] = sep->keyvals ;
	cchar		*vp ;
	if (pip == NULL) return SR_FAULT ;
	if (jep == NULL) return SR_FAULT ;
	if (sep == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcprocprog: ent\n") ;
#endif
	if ((rs = svckv_isprog(kv,n,&vp)) > 0) {
	    cchar	*pfn = jep->ss.var[svckey_p] ;
	    cchar	*astr = jep->ss.var[svckey_a] ;
	    if (hasnonwhite(pfn,-1) || hasnonwhite(astr,-1)) {
	        const int	js = sreqstate_program ;
	        f = TRUE ;
	        if ((rs = sreq_setstate(jep,js)) >= 0) {
	            vecstr	args ;
	            if ((rs = vecstr_start(&args,1,0)) >= 0) {
	                if ((rs = vecstr_srvargs(&args,astr)) >= 0) {
	                    if ((pfn == NULL) || (pfn[0] == '\0')) {
	                        rs = vecstr_get(&args,0,&pfn) ;
	                    }
	                    if (rs >= 0) {
	                        vecstr	*alp = &args ;
	                        rs = mfswatch_svcprocproger(pip,jep,pfn,alp) ;
	                    } /* end if (ok) */
	                } /* end if (vecstr_srvargs) */
	                rs1 = vecstr_finish(&args) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (vecstr) */
	        } /* end if (sreq_setstate) */
	    } /* end if (non-white) */
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcprocprog: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ?  f : rs ;
}
/* end subroutine (mfswatch_svcprocprog) */


static int mfswatch_svcprocproger(PROGINFO *pip,SREQ *jep,
		cchar *pfn,vecstr *alp)
{
	int		rs ;
	char		pbuf[MAXPATHLEN+1] ;
	if ((rs = mfswatch_pathfindbin(pip,pbuf,pfn,-1)) >= 0) {
	    if ((rs = mfswatch_progspawn(pip,jep,pbuf,alp)) >= 0) {
	        rs = sreq_exiting(jep) ;
	    }
	} else if (isNotPresent(rs)) {
	    cchar	*fmt = "program file not found (%d)" ;
	    logssprintf(pip,jep->logid,fmt,rs) ;
	    rs = SR_OK ;
	}
	return rs ;
}
/* end subroutine (mfswatch_svcprocproger) */


static int mfswatch_progspawn(PROGINFO *pip,SREQ *jep,cchar *pbuf,vecstr *alp)
{
	MFSWATCH	*wip = pip->watch ;
	LOCINFO		*lip = pip->lip ;
	int		rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("mfswatch_progspawn: ent\n") ;
	    debugprintf("mfswatch_progspawn: pbuf=%s\n",pbuf) ;
	}
#endif
	if ((rs = locinfo_tmpourdir(lip)) >= 0) {
	    cchar	*dname = lip->tmpourdname ;
	    if ((rs = sreq_openstderr(jep,dname)) >= 0) {
	        const int	efd = rs ;
	        if ((rs = sreq_ofd(jep)) >= 0) {
	            const int	ofd = rs ;
		    cchar	**av ;
	            if ((rs = vecstr_getvec(alp,&av)) >= 0) {
	                ENVHELP	*ehp = &wip->eh ;
	                cchar	**ev ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("mfswatch_progspawn: mid1 rs=%d\n",rs) ;
#endif
	                if ((rs = envhelp_getvec(ehp,&ev)) >= 0) {
	                    SPAWNPROC	ps ;
	                    memset(&ps,0,sizeof(SPAWNPROC)) ;
	                    ps.opts = 0 ;
	                    ps.opts |= SPAWNPROC_OSETSID ;
	                    ps.opts |= SPAWNPROC_OIGNINTR ;
	                    ps.fd[0] = jep->ifd ;
	                    ps.fd[1] = ofd ;
	                    ps.fd[2] = efd ;
	                    ps.disp[0] = SPAWNPROC_DDUP ;
	                    ps.disp[1] = SPAWNPROC_DDUP ;
	                    ps.disp[2] = SPAWNPROC_DDUP ;
	                    if ((rs = spawnproc(&ps,pbuf,av,ev)) >= 0) {
	                        cchar	*pn = pip->progname ;
	                        cchar	*fmt = "spawned pid=%u" ;
	                        cchar	*lid = jep->logid ;
	                        jep->f.process = TRUE ;
	                        jep->pid = rs ;
	                        wip->nprocs += 1 ;
	                        logssprintf(pip,lid,fmt,rs) ;
	                        fmt = "pf=%s" ;
	                        logssprintf(pip,lid,fmt,pbuf) ;
	                        if (pip->debuglevel > 0) {
	                            const int	jsn = jep->jsn ;
	                            fmt = "%s: jsn=%u spawned pid=%u\n" ;
	                            shio_printf(pip->efp,fmt,pn,jsn,rs) ;
	                            fmt = "%s: jsn=%u pf=%s" ;
	                            shio_printf(pip->efp,fmt,pn,jsn,pbuf) ;
	                        }
	                    } else if (isNotPresent(rs)) {
	                        cchar	*fmt = "spawn failure (%d)" ;
	                        cchar	*lid = jep->logid ;
	                        rs = logssprintf(pip,lid,fmt,rs) ;
	                    }
    #if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("mfswatch_progspawn: mid3 rs=%d\n",
				    rs) ;
#endif
	                } /* end if (envhelp_getvec) */
	            } /* end if (vecstr_getvec) */
	        } /* end if (sreq_ofd) */
	    } /* end if (sreq_openstderr) */
	} /* end if (locinfo_tmpourdir) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_progspawn: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_progspawn) */


static int mfswatch_builthave(PROGINFO *pip,cchar *svc)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_builthave: ent svc=%s\n",svc) ;
#endif
	if (wip->open.built) {
	    MFSBUILT	*blp = &wip->built ;
	    rs = mfsbuilt_have(blp,svc,-1) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_builthave: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_builthave) */


static int mfswatch_builthandle(PROGINFO *pip,SREQ *jep,cchar **sav)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_builthandle: ent\n") ;
#endif
	if (wip->open.built) {
	    MFSBUILT		*blp = &wip->built ;
	    MFSERVE_INFO	info ;
	    cchar		*svc = jep->svc ;
	    if ((rs = mfsbuilt_loadbegin(blp,&info,svc,-1)) >= 0) {
	        if ((rs = sreq_builtload(jep,&info)) >= 0) {
	            cchar	*pr = pip->pr ;
	            cchar	**envv = pip->envv ;
		    wip->nbuilts += 1 ;
	            rs = sreq_objstart(jep,pr,sav,envv) ;
	        }
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_builthandle: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_builthandle) */


static int mfswatch_svchelp(PROGINFO *pip,SREQ *jep)
{
	svcprocer_t	w = (svcprocer_t) mfswatch_svchelper ;
	int		rs ;
	int		f = FALSE ;
	if (pip == NULL) return SR_FAULT ;
	if (jep == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svchelp: ent\n") ;
#endif
	if ((rs = sreq_sncreate(jep)) >= 0) {
	    if ((rs = mfswatch_loadsvcs(pip,jep)) >= 0) {
	        if ((rs = mfswatch_svcprocer(pip,jep,w)) >= 0) {
	            f = TRUE ;
	        }
	    } /* end if (mfswatch_loadsvcs) */
	    if (rs < 0)
	        sreq_sndestroy(jep) ;
	} /* end if (sreq_sncreate) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svchelp: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfswatch_svchelp) */


/* this runs as an independent thread */
static int mfswatch_svchelper(PROGINFO *pip,SREQ *jep)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = sreq_ofd(jep)) >= 0) {
	    FILEBUF	b ;
	    const int	ofd = rs ;
	    const int	bs = MAXNAMELEN ;
	    const int	fo = 0 ;
	    if ((rs = filebuf_start(&b,ofd,0L,bs,fo)) >= 0) {
	        SREQ_SNCUR	cur ;
	        if ((rs = sreq_snbegin(jep,&cur)) >= 0) {
	            const int	rsn = SR_NOTFOUND ;
	            cchar	*sp ;
	            while ((rs1 = sreq_snenum(jep,&cur,&sp)) >= 0) {
	                rs = filebuf_print(&b,sp,-1) ;
	                wlen += rs ;
	                if (rs < 0) break ;
	            } /* end while */
	            if ((rs >= 0) && (rs1 != rsn)) rs = rs1 ;
	            rs1 = sreq_snend(jep,&cur) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (osetstr-cur) */
	        rs1 = filebuf_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */
	} /* end if (sreq_ofd) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfswatch_svchelper) */


static int mfswatch_loadsvcs(PROGINFO *pip,SREQ *jep)
{
	MFSWATCH	*wip = pip->watch ;
	const int	elen = (5*MAXNAMELEN) ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		*ebuf ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("mfswatch_loadsvcs: ent\n") ;
	    debugprintf("mfswatch_loadsvcs: f_svcfile=%u\n",
		wip->open.svcfile) ;
	    debugprintf("mfswatch_loadsvcs: f_built=%u\n",
		wip->open.built) ;
	}
#endif
	if ((rs = uc_malloc((elen+1),&ebuf)) >= 0) {
	    const int	rsn = SR_NOTFOUND ;
	    if ((rs >= 0) && wip->open.svcfile) {
	        SVCFILE		*sfp = &wip->tabs ;
	        SVCFILE_CUR	cur ;
	        if ((rs = svcfile_curbegin(sfp,&cur)) >= 0) {
	            SVCFILE_ENT 	e ;
	            while ((rs1 = svcfile_enum(sfp,&cur,&e,ebuf,elen)) >= 0) {
#if	CF_DEBUG
		if (DEBUGLEVEL(5))
		debugprintf("mfswatch_loadsvcs: svc=%s\n",e.svc) ;
#endif
	                rs = sreq_snadd(jep,e.svc,-1) ;
	                c += 1 ;
	                if (rs < 0) break ;
	            } /* end while */
	            if ((rs >= 0) && (rs1 != rsn)) rs = rs1 ;
	            rs1 = svcfile_curend(sfp,&cur) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (svcfile-cur) */
	    } /* end if (f_svcfile) */
#if	CF_DEBUG
		if (DEBUGLEVEL(5))
		debugprintf("mfswatch_loadsvcs: mid1 rs=%d\n",rs) ;
#endif
	    if ((rs >= 0) && wip->open.built) {
		MFSBUILT	*blp = &wip->built ;
		MFSBUILT_CUR	cur ;
		if ((rs = mfsbuilt_curbegin(blp,&cur)) >= 0) {
		    while ((rs = mfsbuilt_enum(blp,&cur,ebuf,elen)) > 0) {
#if	CF_DEBUG
		if (DEBUGLEVEL(5))
		debugprintf("mfswatch_loadsvcs: el=%u ebuf=%s\n",rs,ebuf) ;
#endif
	            	rs = sreq_snadd(jep,ebuf,rs) ;
	                c += 1 ;
			if (rs < 0) break ;
		    } /* end while */
#if	CF_DEBUG
		if (DEBUGLEVEL(5))
		debugprintf("mfswatch_loadsvcs: "
			"mfsbuilt_curbegin while-out rs=%d\n",rs) ;
#endif
		    rs1 = mfsbuilt_curend(blp,&cur) ;
	    	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUG
		if (DEBUGLEVEL(5))
		debugprintf("mfswatch_loadsvcs: "
			"mfsbuilt_curbegin mfsbuilt_curend() rs=%d\n",rs) ;
#endif
		} /* end if (mfsbuilt-cur) */
#if	CF_DEBUG
		if (DEBUGLEVEL(5))
		debugprintf("mfswatch_loadsvcs: "
			"mfsbuilt_curbegin out rs=%d\n",rs) ;
#endif
	    } /* end if (f_built) */
	    rs1 = uc_free(ebuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfswatch_loadsvcs: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfswatch_loadsvcs) */


/* this spawns a thread with the required arguments */
static int mfswatch_svcprocer(PROGINFO *pip,SREQ *jep,svcprocer_t w)
{
	MFSWATCH	*wip = pip->watch ;
	const int	js = sreqstate_thread ;
	int		rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcprocer: ent\n") ;
#endif
	if ((rs = sreq_setstate(jep,js)) >= 0) {
	    const int	size = sizeof(SVCPROCARGS) ;
	    SVCPROCARGS	*sap ;
	    if ((rs = uc_malloc(size,&sap)) >= 0) {
	        uptsub_t	helper = (uptsub_t) svcprocers ;
	        thread_t	tid ;
	        sap->pip = pip ;
	        sap->jep = jep ;
	        sap->w = w ;
	        if ((rs = uptcreate(&tid,NULL,helper,sap)) >= 0) {
	            jep->tid = tid ;
	            jep->f.thread = TRUE ;
	            wip->nthrs += 1 ;
	        }
	    } /* end if (m-a) */
	} /* end if (sreq_setstate) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcprocer: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_svcprocer) */


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


/* returns connect status to the client */
static int mfswatch_svcretstat(PROGINFO *pip,SREQ *jep,int f)
{
	LOCINFO		*lip = pip->lip ;
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		ofd = jep->ofd ;
	int		len = 0 ;
	int		svctype ;
	cchar		*fmt = NULL ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_svcretstatus: ent f=%u\n",f) ;
#endif
	if (wip == NULL) return SR_FAULT ;
	if (ofd < 0) ofd = jep->ifd ;
	svctype = (lip->svctype >= 0) ? lip->svctype : pip->progmode ;
	switch (svctype) {
	case svctype_mfserve:
	case svctype_tcpmux:
	    if (f > 0) {
	        fmt = "+ (%u)\n" ;
	    } else {
	        fmt = "- service not found (%d)\n" ;
	    }
	    break ;
	} /* end switch */
	if (fmt != NULL) {
	    const int	rlen = TIMEBUFLEN ;
	    char	rbuf[TIMEBUFLEN+1] ;
	    if ((rs = bufprintf(rbuf,rlen,fmt,f)) >= 0) {
	        if ((rs = uc_writen(ofd,rbuf,rs)) >= 0) {
	            len = rs ;
	        } else if (isBadSend(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (bufprintf) */
	} /* end if (response required) */
	if ((rs >= 0) && (f <= 0)) {
	    cchar	*fmt = "service not found" ;
	    logssprintf(pip,jep->logid,fmt) ;
	}
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mfswatch_svcretstat) */


static int mfswatch_jobretire(PROGINFO *pip,SREQ *jep)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_jobretire: ent {%p}\n",jep) ;
#endif
	if ((rs = sreq_svcentend(jep)) >= 0) {
	    SREQDB	*dbp = &wip->reqs ;
	    cchar	*fmt = "%s done" ;
	    char	tbuf[TIMEBUFLEN+1] ;
	    timestr_logz(pip->daytime,tbuf) ;
	    logssprintf(pip,jep->logid,fmt,tbuf) ;
	    rs = sreqdb_delobj(dbp,jep) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_jobretire: ret rs=%d \n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_jobretire) */


static int mfswatch_checkthrs(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		c = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_checkthrs: ent nthrs=%u\n",wip->nthrs) ;
#endif
	if (wip->nthrs > 0) {
	    SREQDB	*sdp = &wip->reqs ;
	    SREQ	*jep ;
	    const int	rsn = SR_NOTFOUND ;
	    int		f_loop = TRUE ;
	    while ((rs >= 0) && f_loop) {
	        if ((rs = sreqdb_thrsdone(sdp,&jep)) >= 0) {
	            c += 1 ;
	            rs = mfswatch_thrdone(pip,jep) ;
	        } else if (rs == rsn) {
	            f_loop = FALSE ;
	            rs = SR_OK ;
	        }
	    } /* end while */
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_checkthrs: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfswatch_checkthrs) */


/* ARGSUSED */
static int mfswatch_thrdone(PROGINFO *pip,SREQ *jep)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs ;
	if (wip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_thrdone: ent\n") ;
#endif
	if ((rs = sreq_thrdone(jep)) >= 0) {
	    if (wip->nthrs) wip->nthrs -= 1 ;
	    rs = mfswatch_jobretire(pip,jep) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_thrdone: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfswatch_thrdone) */


static int mfswatch_checkprogs(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		c = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_checkprogs: ent nprocs=%u\n",wip->nprocs) ;
#endif
	if (wip->nprocs > 0) {
	    int		cs ;
	    if ((rs = u_waitpid(-1,&cs,W_OPTIONS)) > 0) {
	        SREQDB		*slp = &wip->reqs ;
	        SREQ		*jep ;
	        const pid_t	pid = rs ;
	        if ((rs = sreqdb_findpid(slp,pid,&jep)) >= 0) {
	            const int	jsn = jep->jsn ;
	            cchar	*lid = jep->logid ;
	            if ((rs = mfswatch_logprogres(pip,jsn,lid,pid,cs)) >= 0) {
	                if ((rs = mfswatch_logprogchild(pip,jep)) >= 0) {
	                    if (wip->nprocs) wip->nprocs -= 1 ;
	                    c += 1 ;
	                    rs = mfswatch_jobretire(pip,jep) ;
	                }
	            }
	        } else if (rs == SR_NOTFOUND) {
	            const int	jsn = -1 ;
	            cchar	*lid = "orphan" ;
	            if ((rs = mfswatch_logprogres(pip,jsn,lid,pid,cs)) >= 0) {
	                logprintf(pip,"orphan subprocess pid=%u",pid) ;
	            }
	        }
	    } else if ((rs == SR_CHILD) || (rs == SR_INTR)) {
	        rs = SR_OK ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_checkprogs: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfswatch_checkprogs) */


static int mfswatch_checkbuilts(PROGINFO *pip)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_checkbuilts: ent\n") ;
#endif
	if (wip->open.built) {
	    SREQDB	*sdp = (SREQDB *) &wip->reqs ;
	    SREQ	*jep ;
	    const int	rsn = SR_NOTFOUND ;
	    int		i ;
	    for (i = 0 ; (rs1 = sreqdb_get(sdp,i,&jep)) >= 0 ; i += 1) {
	        if ((jep != NULL) && (jep->objp != NULL)) {
		    rs = mfswatch_checkbuilt(pip,jep) ;
		    c += rs ;
		} /* end if (selection) */
		if (rs < 0) break ;
	    } /* end for */
	    if ((rs >= 0) && (rs1 != rsn)) rs = rs1 ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_checkbuilts: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfswatch_checkbuilts) */


static int mfswatch_checkbuilt(PROGINFO *pip,SREQ *jep)
{
	MFSWATCH	*wip = pip->watch ;
	int		rs ;
	int		c = 0 ;
	if ((rs = sreq_objcheck(jep)) > 0) {
	    if ((rs = sreq_objfinish(jep)) >= 0) {
		if ((rs = mfswatch_logprogchild(pip,jep)) >= 0) {
		    MFSBUILT	*blp = &wip->built ;
		    const int	slen = SVCNAMELEN ;
		    char	sbuf[SVCNAMELEN+1] ;
		    strdcpy1(sbuf,slen,jep->svc) ;
		    if (wip->nbuilts) wip->nbuilts -= 1 ;
		    c += 1 ;
		    if ((rs = mfswatch_jobretire(pip,jep)) >= 0) {
			rs = mfsbuilt_loadend(blp,sbuf,-1) ;
		    }
		}
	    }
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfswatch_checkbuilt) */


static int mfswatch_logprogres(PROGINFO *pip,int jsn,cchar *lid,
		pid_t pid,int cs)
{
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		timebuf[TIMEBUFLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_logprogres: ent jsn=%d lid=%s pid=%u\n",
	        jsn,lid,pid) ;
#endif

	if (lid == NULL) lid = "orgphan" ;
	timestr_logz(pip->daytime,timebuf) ;

	if (WIFEXITED(cs)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("mfswatch_logprogres: normal ex=%u\n",
	            WEXITSTATUS(cs)) ;
#endif

	    if (pip->debuglevel > 0) {
	        shio_printf(pip->efp,"hello world!\n") ;
	        fmt = "%s: jsn=%d server(%u) exited normally ex=%u\n" ;
	        shio_printf(pip->efp,fmt,pn,jsn,pid,WEXITSTATUS(cs)) ;
	    }

	    if (pip->open.logprog) {
	        fmt = "%s server(%u) exited normally ex=%u" ;
	        logssprintf(pip,lid,fmt,timebuf,pid,WEXITSTATUS(cs)) ;
	    }

	} else if (WIFSIGNALED(cs)) {
	    const int	sig = WTERMSIG(cs) ;
	    cchar	*ss ;
	    char	sigbuf[20+1] ;

	    if ((ss = strsigabbr(sig)) == NULL) {
	        ctdeci(sigbuf,20,sig) ;
	        ss = sigbuf ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progwatchjobs: signalled sig=%s\n",ss) ;
#endif

	    if (! pip->f.quiet) {
	        fmt = "%s: jsn=%d server(%u) was signalled sig=%s\n" ;
	        shio_printf(pip->efp,fmt,pn,jsn,pid,ss) ;
	    }

	    if (pip->open.logprog) {
	        fmt = "%s server(%u) was signalled sig=%s" ;
	        logssprintf(pip,lid,fmt,timebuf,pid,ss) ;
	    }

	} else {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("mfswatch_logprogres: abnormal cs=%u\n",cs) ;
#endif

	    if (! pip->f.quiet) {
	        fmt = "%s: jsn=%d server(%u) exited abnormally cs=%u\n" ;
	        shio_printf(pip->efp,fmt,pn,jsn,pid,cs) ;
	    }

	    if (pip->open.logprog) {
	        fmt = "%s server(%u) exited abnormally cs=%u" ;
	        logssprintf(pip,lid,fmt,timebuf,pid,cs) ;
	    }

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_logprogres: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfswatch_logprogres) */


static int mfswatch_logprogchild(PROGINFO *pip,SREQ *jep)
{
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    cchar	*lid = jep->logid ;
	    char	timebuf[TIMEBUFLEN+1] ;
	    rs = logoutfile(pip,lid,"stderr",jep->efname) ;
	    timestr_elapsed((pip->daytime - jep->stime), timebuf) ;
	    logssprintf(pip,lid,"elapsed time %s\n",timebuf) ;
	} /* end if (have logging) */
	return rs ;
}
/* end subroutine (mfswatch_logprogchild) */


/* log a connection */
static int mfswatch_logconn(PROGINFO *pip,int jsn,int jt,int st,cchar *lid)
{
	int		rs = SR_OK ;
	cchar		*fmt ;
	char		tbuf[TIMEBUFLEN+1] ;
	fmt = "%s connect jtype=%u stype=%u" ;
	timestr_logz(pip->daytime,tbuf) ;
	logssprintf(pip,lid,fmt,tbuf,jt,st) ;
	return rs ;
}
/* end subroutine (mfswatch_logconn) */


static int mfswatch_logsvc(PROGINFO *pip,SREQ *jep)
{
	int		rs ;
	cchar		*svc ;
	if ((rs = sreq_getsvc(jep,&svc)) >= 0) {
	    cchar	*fmt = "svc=%s ss=%s\n" ;
	    logssprintf(pip,jep->logid,fmt,jep->svc,jep->subsvc) ;
	}
	return rs ;
}
/* end subroutine (mfswatch_logconn) */


static int svcprocers(SVCPROCARGS *sap)
{
	PROGINFO	*pip = sap->pip ;
	SREQ		*jep = sap->jep ;
	SVCPROCARGS	sa = *sap ;
	int		rs ;
	{
	    MFSWATCH	*wip = pip->watch ;
	    if ((rs = uc_free(sap)) >= 0) {
	        SREQDB	*srp = &wip->reqs ;
	        sa.jep->trs = (*sa.w)(sa.pip,sa.jep) ;
	        rs = sreqdb_exiting(srp,jep->ji) ;
	    }
	}
	return rs ;
}
/* end subroutine (svcprocers) */


