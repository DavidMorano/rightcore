/* maintqotd */

/* open a channel (file-descriptor) to the quote-of-the-day (QOTD) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_OPENDEF	0		/* ? */
#define	CF_SOURCES	1		/* use sources */
#define	CF_CONFIGCHECK	0		/* |config_check()| */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We sort of form the back-end maintenance end of the QOTD mechansim.

	Synopsis:

	int maintqotd(pr,mjd,of,to)
	cchar		*pr ;
	int		mjd ;
	int		of ;
	int		to ;

	Arguments:

	pr		program-root
	mjd		modified-julian-day
	of		open-flags
	to		time-out

	Returns:

	<0		error
	>=0		FD of QOTD

	Notes:

	- open flags:
		O_NOTTY
		O_EXCL		
		O_SYNC
		O_NDELAY
		O_NONBLOCK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<estrings.h>
#include	<mkfnamesuf.h>
#include	<ids.h>
#include	<sigman.h>
#include	<tmtime.h>
#include	<storebuf.h>
#include	<vecstr.h>
#include	<vecpstr.h>
#include	<ascii.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"maintqotd.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VTMPDNAME
#define	VTMPDNAME	"/var/tmp"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	LOGCNAME
#define	LOGCNAME	"log"
#endif

#define	CONFIGFNAME	"conf"
#define	QCNAME		"qotd"

#ifndef	MAINTQOTD_SEARCHNAME
#define	MAINTQOTD_SEARCHNAME	"maintqotd"
#endif
#ifndef	MAINTQOTD_PROGEXEC
#define	MAINTQOTD_PROGEXEC	"qotd"
#endif
#ifndef	MAINTQOTD_VARSPOOL
#define	MAINTQOTD_VARSPOOL	"var/spool"
#endif

#define	CONFIG		struct config


/* external subroutines */

extern int	snfilemode(char *,int,mode_t) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecpi(char *,int,int,int) ;
extern int	getaf(cchar *,int) ;
extern int	getmjd(int,int,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getprogpath(IDS *,vecstr *,char *,cchar *,int) ;
extern int	getprogexec(char *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	mklogid(char *,int,cchar *,int,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	vecstr_addpathclean(vecstr *,cchar *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	prmktmpdir(cchar *,char *,cchar *,cchar *,mode_t) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isprintlatin(int) ;

extern int	maintqotd_prog(MAINTQOTD *,cchar *,cchar *) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strwcpyuc(char *,cchar *,int) ;
extern char	*strcpylc(char *,cchar *) ;
extern char	*strcpyuc(char *,cchar *) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern cchar	**environ ;


/* local structures */

struct config {
	uint		magic ;
	MAINTQOTD	*sip ;
	PARAMFILE	p ;
	EXPCOOK		cooks ;
	uint		f_p:1 ;
	uint		f_cooks:1 ;
} ;


/* forward references */

static int	subinfo_start(MAINTQOTD *,time_t,cchar *,int,int,int) ;
static int	subinfo_finish(MAINTQOTD *) ;
static int	subinfo_confbegin(MAINTQOTD *) ;
static int	subinfo_confend(MAINTQOTD *) ;
static int	subinfo_setentry(MAINTQOTD *,cchar **,cchar *,int) ;
static int	subinfo_envbegin(MAINTQOTD *) ;
static int	subinfo_envend(MAINTQOTD *) ;
static int	subinfo_logfile(MAINTQOTD *,cchar *,int) ;
static int	subinfo_hostname(MAINTQOTD *,cchar *,int) ;
static int	subinfo_source(MAINTQOTD *,cchar *,int) ;
static int	subinfo_logbegin(MAINTQOTD *) ;
static int	subinfo_logend(MAINTQOTD *) ;
static int	subinfo_logenv(MAINTQOTD *) ;
static int	subinfo_defaults(MAINTQOTD *) ;
static int	subinfo_spooldir(MAINTQOTD *,cchar *,int) ;
static int	subinfo_spoolcheck(MAINTQOTD *) ;
static int	subinfo_qdirname(MAINTQOTD *,int) ;
static int	subinfo_gather(MAINTQOTD *,cchar *,mode_t) ;
static int	subinfo_opensource(MAINTQOTD *,cchar *,cchar *) ;
static int	subinfo_opensourceprog(MAINTQOTD *,cchar *,cchar *) ;

static int subinfo_defprog(MAINTQOTD *,cchar *) ;
static int subinfo_defproger(MAINTQOTD *,vecstr *,cchar *,cchar *) ;
static int subinfo_addourpath(MAINTQOTD *,vecstr *) ;
static int subinfo_addprbins(MAINTQOTD *,vecstr *) ;
static int subinfo_addprbin(MAINTQOTD *,vecstr *,cchar *,cchar *) ;
static int subinfo_id(MAINTQOTD *) ;
static int subinfo_dircheck(MAINTQOTD *,cchar *) ;
static int subinfo_dirminmode(MAINTQOTD *,cchar *,mode_t) ;

static int	config_start(struct config *,MAINTQOTD *,cchar *) ;
static int	config_findfile(struct config *,char *,cchar *) ;
static int	config_cookbegin(struct config *) ;
static int	config_cookend(struct config *) ;
static int	config_read(struct config *) ;
static int	config_reader(struct config *,char *,int) ;
static int	config_finish(struct config *) ;

#if	CF_CONFIGCHECK
static int	config_check(struct config *) ;
#endif

static int	getdefmjd(time_t) ;
static int	mkqfname(char *,cchar *,int) ;

static int	setfname(MAINTQOTD *,char *,cchar *,int,
			int,cchar *,cchar *,cchar *) ;

static int	mkourname(char *,cchar *,cchar *,cchar *,int) ;

#if	CF_DEBUGS && CF_OPENDEF
static int opendef(MAINTQOTD *) ;
#endif

#if	CF_DEBUGS
static int debugmode(cchar *,cchar *,cchar *) ;
#endif

#if	CF_DEBUGS
static int debugfmode(cchar *,cchar *,int) ;
#endif


/* local variables */

static cchar	*csched[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static cchar	*cparams[] = {
	"spooldir",
	"logsize",
	"logfile",
	"hostname",
	"svcname",
	"to",
	"source",
	NULL
} ;

enum cparams {
	cparam_spooldir,
	cparam_logsize,
	cparam_logfile,
	cparam_hostname,
	cparam_svcname,
	cparam_to,
	cparam_source,
	cparam_overlast
} ;

static cchar	*sources[] = {
	"prog",
	"systems",
	"uqotd",
	NULL
} ;

enum sources {
	source_prog,
	source_systems,
	source_uqotd,
	source_overlast
} ;

static cchar	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;

static cchar	*defprogs[] = {
	"mkqotd",
	"fortune",
	"/swd/local/bin/fortune",
	"/usr/extra/bin/fortune",
	"/usr/games/fortune",
	NULL
} ;


/* exported subroutines */


int maintqotd(cchar *pr,int mjd,int of,int to)
{
	MAINTQOTD	si, *sip = &si ;
	time_t		dt = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		fd = -1 ;

	if (pr == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("maintqotd: ent mjd=%d\n",mjd) ;
#endif /* CF_DEBUGS */

	if (mjd <= 0) {
	    if (dt == 0) dt = time(NULL) ;
	    rs = getdefmjd(dt) ;
	    mjd = rs ;
	}

	if (rs >= 0) {
	    if ((rs = subinfo_start(sip,dt,pr,of,to,mjd)) >= 0) {
	        if ((rs = subinfo_defaults(sip)) >= 0) {
	            if ((rs = subinfo_logbegin(sip)) >= 0) {
	                if ((rs = subinfo_spoolcheck(sip)) >= 0) {
	                    if ((rs = subinfo_qdirname(sip,mjd)) >= 0) {
	                        cchar	*qd = sip->qdname ;
	                        char	qfname[MAXPATHLEN+1] ;
	                        if ((rs = mkqfname(qfname,qd,mjd)) >= 0) {
	                            const mode_t	om = 0664 ;

#if	CF_DEBUGS
				    debugprintf("maintqotd: qf=%s\n",qfname) ;
#endif /* CF_DEBUGS */

	                            of &= (~ OM_SPECIAL) ;
    
#if	CF_DEBUGS
				    {
				        char	obuf[TIMEBUFLEN+1] ;
				        snopenflags(obuf,TIMEBUFLEN,of) ;
				        debugprintf("maintqotd: of=%s\n",obuf) ;
				    }
#endif /* CF_DEBUGS */
	                            rs = u_open(qfname,of,om) ;
	                            fd = rs ;
#if	CF_DEBUGS
				    debugprintf("maintqotd: u_open() rs=%d\n",
					rs) ;
#endif /* CF_DEBUGS */
	                            if (rs == SR_NOENT) {
	                                rs = subinfo_gather(sip,qfname,om) ;
	                                fd = rs ;
					if (rs < 0) {
					    uc_unlink(qfname) ;
					}
	                            } /* end if (NOENT) */

	                        } /* end if (mkqfname) */
			    } /* end if (qdirname) */
	                } /* end if (spoolcheck) */
	                rs1 = subinfo_logend(sip) ;
	        	if (rs >= 0) rs = rs1 ;
		    } /* end if (logging) */
	        } /* end if (defaults) */
	        rs1 = subinfo_finish(sip) ;
	        if (rs >= 0) rs = rs1 ;
	        if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	    } /* end if (subinfo) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("maintqotd: ret rs=%d fd=%u\n",rs,fd) ;
#endif /* CF_DEBUGS */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (maintqotd) */


/* local subroutines */


static int subinfo_start(MAINTQOTD *sip,time_t dt,cchar *pr,
		int of,int to,int mjd)
{
	struct ustat	sb ;
	int		rs ;

	if (dt == 0) dt = time(NULL) ;

#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_start: ent pr=%s\n",pr) ;
#endif

	memset(sip,0,sizeof(MAINTQOTD)) ;
	sip->pr = pr ;
	sip->of = of ;
	sip->om = 0666 ;
	sip->to = to ;
	sip->mjd = mjd ;
	sip->sn = MAINTQOTD_SEARCHNAME ;
	sip->dt = dt ;
	sip->euid = geteuid() ;

	sip->f.logsub = TRUE ;

	if ((rs = u_stat(pr,&sb)) >= 0) {
	    sip->uid_pr = sb.st_uid ;
	    sip->gid_pr = sb.st_gid ;

	    if ((rs = subinfo_envbegin(sip)) >= 0) {
	        if ((rs = subinfo_confbegin(sip)) >= 0) {
		    const int	llen = LOGIDLEN ;
		    const int	v = (int) ugetpid() ;
		    cchar	*nn = sip->nn ;
		    char	lbuf[LOGIDLEN+1] ;
		    if ((rs = mklogid(lbuf,llen,nn,5,v)) >= 0) {
			cchar	**vpp = &sip->logid ;
			rs = subinfo_setentry(sip,vpp,lbuf,rs) ;
		    }
		    if (rs < 0) {
	        	subinfo_confend(sip) ;
		    }
		}
	        if (rs < 0)
	            subinfo_envend(sip) ;
	    }
	    if (rs < 0) {
	        if (sip->open.stores) {
	            sip->open.stores = FALSE ;
	            vecstr_finish(&sip->stores) ;
	        }
	    }

	} /* end if (stat-pr) */

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(MAINTQOTD *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = subinfo_confend(sip) ;
	if (rs >= 0) rs = rs1 ;

	if (sip->open.sources) {
	    sip->open.sources = FALSE ;
	    rs1 = vecpstr_finish(&sip->sources) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->open.hosts) {
	    sip->open.hosts = FALSE ;
	    rs1 = vecpstr_finish(&sip->hosts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->open.id) {
	    sip->open.id = FALSE ;
	    rs1 = ids_release(&sip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->open.stores) {
	    sip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&sip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


int subinfo_setentry(MAINTQOTD *lip,cchar **epp,cchar vp[],int vl)
{
	int		rs = SR_OK ;
	int		vnlen = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;

	    if (*epp != NULL)
	        oi = vecstr_findaddr(&lip->stores,*epp) ;

	    if (vp != NULL) {
	        vnlen = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,vnlen,epp) ;
	    } else
		*epp = NULL ;

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&lip->stores,oi) ;

	} /* end if (ok) */

	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (subinfo_setentry) */


static int subinfo_envbegin(MAINTQOTD *sip)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;

#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_envbegin: ent\n") ;
#endif

	{
	    const int	elen = MAXPATHLEN ;
	    int		el = -1 ;
	    cchar	*en ;
	    char	ebuf[MAXPATHLEN+1] ;
	    en = ebuf ;
	    if ((rs = getprogexec(ebuf,elen)) == SR_NOSYS) {
	        rs = SR_OK ;
	        en = MAINTQOTD_PROGEXEC ;
	    } else
	        el = rs ;
	    if (rs >= 0) {
	        cchar	**vpp = &sip->pn ;
	        if ((cl = sfbasename(en,el,&cp)) > 0) {
		    cchar	*tp ;
		    if ((tp = strnchr(cp,cl,'.')) != NULL) cl = (tp-cp) ;
	            rs = subinfo_setentry(sip,vpp,cp,cl) ;
	        }
	    }
	}

	if (rs >= 0) {
	    char	nn[NODENAMELEN+1] ;
	    char	dn[MAXHOSTNAMELEN+1] ;
	    if ((rs = getnodedomain(nn,dn)) >= 0) {
	        cchar	**vpp = &sip->nn ;
#if	CF_DEBUGS
	        debugprintf("maintqotd/subinfo_envbegin: nn=%s\n",nn) ;
	        debugprintf("maintqotd/subinfo_envbegin: dn=%s\n",dn) ;
#endif
	        if ((rs = subinfo_setentry(sip,vpp,nn,-1)) >= 0) {
	            cchar	**vpp = &sip->dn ;
	            rs = subinfo_setentry(sip,vpp,dn,-1) ;
	        }
	    }
	}

	if (rs >= 0) {
	    const int	ulen = USERNAMELEN ;
	    char	ubuf[USERNAMELEN+1] ;
	    if ((rs = getusername(ubuf,ulen,-1)) >= 0) {
	        cchar	**vpp = &sip->un ;
	        rs = subinfo_setentry(sip,vpp,ubuf,rs) ;
	    }
	}

	return rs ;
}
/* end subroutine (subinfo_envbegin) */


static int subinfo_envend(MAINTQOTD *sip)
{
	int		rs = SR_OK ;

	if (sip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (subinfo_envend) */


static int subinfo_confbegin(MAINTQOTD *sip)
{
	const int	csize = sizeof(struct config)  ;
	int		rs = SR_OK ;
	cchar		*cfname = CONFIGFNAME ;
	void		*p ;

#if	CF_DEBUGS
	    debugprintf("maintqotd/subinfo_confbegin: ent\n") ;
#endif

	if ((rs = uc_malloc(csize,&p)) >= 0) {
	    CONFIG	*csp = p ;
	    sip->config = csp ;
	    if ((rs = config_start(csp,sip,cfname)) >= 0) {
	        if ((rs = config_read(csp)) >= 0) {
	            rs = 1 ;
	        }
	        if (rs < 0)
	            config_finish(csp) ;
	    } /* end if (config) */
	    if (rs < 0) {
	        uc_free(p) ;
	        sip->config = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	    debugprintf("maintqotd/subinfo_confbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_confbegin) */


static int subinfo_confend(MAINTQOTD *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	    debugprintf("maintqotd/subinfo_confend: config=%u\n",
	        (sip->config != NULL)) ;
#endif

	if (sip->config != NULL) {
	    CONFIG	*csp = sip->config ;
	    rs1 = config_finish(csp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(sip->config) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->config = NULL ;
	}

	return rs ;
}
/* end subroutine (subinfo_confend) */


static int subinfo_defaults(MAINTQOTD *sip)
{
	int		rs = SR_OK ;

	if (sip->spooldname == NULL) {
	    cchar	*vp = sip->sn ;
	    const int	vl = -1 ;
	    rs = subinfo_spooldir(sip,vp,vl) ;
	}

	{
	    cchar	*lf = sip->lfname ;
	    if (((lf == NULL) || (lf[0] == '+')) && sip->f.logsub) {
	        cchar	*vp = sip->sn ;
	        const int	vl = -1 ;
	        rs = subinfo_logfile(sip,vp,vl) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("subinfo_defaults: lfname=%s\n",sip->lfname) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_defaults) */


static int subinfo_spooldir(MAINTQOTD *sip,cchar *vp,int vl)
{
	int		rs ;
	cchar		*pr = sip->pr ;
	cchar		*inter = MAINTQOTD_VARSPOOL ;
	char		tbuf[MAXPATHLEN+1] ;

	if ((rs = mkourname(tbuf,pr,inter,vp,vl)) >= 0) {
	    cchar	**vpp = &sip->spooldname ;
	    rs = subinfo_setentry(sip,vpp,tbuf,rs) ;
	}

	return rs ;
}
/* end subroutine (subinfo_spooldir) */


static int subinfo_logfile(MAINTQOTD *sip,cchar *vp,int vl)
{
	int		rs ;
	cchar		*pr = sip->pr ;
	cchar		*inter = LOGCNAME ;
	char		tbuf[MAXPATHLEN+1] ;

	if ((rs = mkourname(tbuf,pr,inter,vp,vl)) >= 0) {
	    cchar	**vpp = &sip->lfname ;
	    rs = subinfo_setentry(sip,vpp,tbuf,rs) ;
	}

	return rs ;
}
/* end subroutine (subinfo_logfile) */


static int subinfo_hostname(MAINTQOTD *sip,cchar *vp,int vl)
{
	int	rs = SR_OK ;

	if (! sip->open.hosts)  {
	    rs = vecpstr_start(&sip->hosts,0,0,0) ;
	    sip->open.hosts = (rs >= 0) ;
	}

	if ((rs >= 0) && (vp != NULL)) {
	    rs = vecpstr_adduniq(&sip->hosts,vp,vl) ;
	}

	return rs ;
}
/* end subroutine (subinfo_hostname) */


static int subinfo_source(MAINTQOTD *sip,cchar *vp,int vl)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_source: ent v=>%t<\n",vp,vl) ;
#endif

	if (! sip->open.sources)  {
	    rs = vecpstr_start(&sip->sources,0,0,0) ;
	    sip->open.sources = (rs >= 0) ;
	}

	if ((rs >= 0) && (vp != NULL)) {
	    rs = vecpstr_adduniq(&sip->sources,vp,vl) ;
	}

	return rs ;
}
/* end subroutine (subinfo_source) */


static int subinfo_logbegin(MAINTQOTD *sip)
{
	int		rs = SR_OK ;
	cchar		*lf = sip->lfname ;

	if ((lf != NULL) && (lf[0] != '-')) {
	    const int	size = sizeof(LOGFILE) ;
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
		LOGFILE	*lhp = p ;
	        cchar	*logid = sip->logid ;
	        sip->logsub = p ;
#if	CF_DEBUGS
	debugprintf("maintqotd/_logbegin: lf=%s\n",lf) ;
	debugprintf("maintqotd/_logbegin: logid=%s\n",logid) ;
#endif /* CF_DEBUGS */
	        if ((rs = logfile_open(lhp,lf,0,0666,logid)) >= 0) {
		    sip->open.logsub = TRUE ;
		    rs = subinfo_logenv(sip) ;
		    if (rs < 0) {
			sip->open.logsub = FALSE ;
			logfile_close(lhp) ;
		    }
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
		}
#if	CF_DEBUGS
	debugprintf("maintqotd/_logbegin: logfile_open-out rs=%d\n",rs) ;
#endif /* CF_DEBUGS */
		if (rs < 0) {
		    uc_free(sip->logsub) ;
		    sip->logsub = NULL ;
		}
	    } /* end if (memory-allocation) */
	} /* end if (log-file) */

#if	CF_DEBUGS
	debugprintf("maintqotd/_logbegin: ret rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (subinfo_logbegin) */


static int subinfo_logend(MAINTQOTD *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->logsub != NULL) {
	    LOGFILE	*lhp = sip->logsub ;
	    rs1 = logfile_close(lhp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(sip->logsub) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->logsub = NULL ;
	}

	return rs ;
}
/* end subroutine (subinfo_logend) */


static int subinfo_logenv(MAINTQOTD *sip)
{
	int		rs = SR_OK ;
	char		tbuf[TIMEBUFLEN+1] ;

	if (sip->open.logsub) {
	    LOGFILE	*lhp = sip->logsub ;
	    timestr_logz(sip->dt,tbuf) ;
	    logfile_printf(lhp,"%s %s %s!%s",tbuf,sip->dn,sip->nn,sip->un) ;
#ifdef	COMMENT
	    logfile_printf(lhp,"pr=%s",sip->pr) ;
	    logfile_printf(lhp,"pn=%s",sip->pn) ;
#endif /* COMMENT */
	    logfile_printf(lhp,"mjd=%d",sip->mjd) ;
	}

	return rs ;
}
/* end subroutine (subinfo_logenv) */


static int subinfo_spoolcheck(MAINTQOTD *sip)
{
	int		rs ;
	cchar		*sdname = sip->spooldname ;
	rs = subinfo_dircheck(sip,sdname) ;
	return rs ;
}
/* end subroutine (subinfo_spoolcheck) */


static int subinfo_qdirname(MAINTQOTD *sip,int mjd)
{
	const int	dlen = DIGBUFLEN ;
	const int	prec = 3 ; /* digit precision for another 100 years */
	int		rs ;
	int		len = 0 ;
	cchar		*sdname = sip->spooldname ;
	char		dbuf[DIGBUFLEN+1] ;

#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_qdirname: ent mjd=%u\n",mjd) ;
#endif

	if ((rs = ctdecpi((dbuf+1),(dlen-1),prec,(mjd/100))) > 0) {
	    char	tbuf[MAXPATHLEN+1] ;
	    dbuf[0] = 'd' ;
	    if ((rs = mkpath2w(tbuf,sdname,dbuf,(rs+1))) >= 0) {
	        cchar	**vpp = &sip->qdname ;
		len = rs ;
		if ((rs = subinfo_setentry(sip,vpp,tbuf,len)) >= 0) {
		    rs = subinfo_dircheck(sip,tbuf) ;
		} /* end if (subinfo_setentry) */
	    } /* end if (mkpath) */
	} /* end if (ctdeci) */

#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_qdirname: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_qdirname) */


#if	CF_SOURCES
static int subinfo_gather(MAINTQOTD *sip,cchar *qfname,mode_t om)
{
	int		rs = SR_OK ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_gather: ent\n") ;
	debugprintf("maintqotd/subinfo_gather: qf=%s\n",qfname) ;
#endif

	if (sip->open.sources) {
	    VECPSTR	*slp = &sip->sources ;
	    int		i ;
	    cchar	*sep ;
#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_gather: n=%d\n",vecpstr_count(slp)) ;
#endif
	    for (i = 0 ; (rs = vecpstr_get(slp,i,&sep)) >= 0 ; i += 1) {
	        if (sep != NULL) {
	            rs = subinfo_opensource(sip,qfname,sep) ;
	            fd = rs ;
	        }
	        if (rs >= 0) break ;
	        if (! isNotPresent(rs)) break ;
	    } /* end for */
	    if (isNotPresent(rs)) {
	        rs = subinfo_defprog(sip,qfname) ;
	        fd = rs ;
	    }
	} else {
	    rs = subinfo_defprog(sip,qfname) ;
	    fd = rs ;
	}
#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_gather: mid rs=%d fd=%u\n",rs,fd) ;
#endif
	if (rs >= 0) {
	    if ((rs = u_rewind(fd)) >= 0) {
		if ((rs = uc_fminmod(fd,om)) >= 0) {
		    uid_t	u = sip->uid_pr ;
		    gid_t	g = sip->gid_pr ;
		    if ((rs = u_fchown(fd,u,g)) == SR_PERM) rs = SR_OK ;
		}
	    } /* end if (rewind) */
	    if (rs < 0) u_close(fd) ;
	} /* end if (got a source) */
#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_gather: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (subinfo_gather) */
#else /* CF_SOURCES */
static int subinfo_gather(MAINTQOTD *sip,cchar *qfname,mode_t om)
{
	const mode_t	om = 0664 ;
	const int	of = (O_RDWR|O_CREAT|O_TRUNC) ;
	int		rs ;
	int		fd = -1 ;
	if ((rs = u_open(qfname,of,om)) >= 0) {
	    cchar	*mp = "hello world!\n" ;
	    int		ml ;
	    fd = rs ;
	    ml = strlen(mp) ;
	    rs = u_write(fd,mp,ml) ;
	    if (rs >= 0) rs = u_rewind(fd) ;
	    if (rs < 0) u_close(fd) ;
	}
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (subinfo_gather) */
#endif /* CF_SOURCES */


static int subinfo_opensource(MAINTQOTD *sip,cchar *qf,cchar *sep)
{
	int		rs = SR_OK ;
	int		fd = -1 ;
	int		sl = -1 ;
	int		si ;
	cchar		*sp = sep ;
	cchar		*ap ;
	cchar		*tp ;
	if ((tp = strchr(sp,CH_FS)) != NULL) {
	    sl = (tp-sp) ;
	    ap = (tp+1) ;
	} else {
	    ap = (sp + strlen(sp)) ;
	}
#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_opensource: svc=%t\n",sp,sl) ;
	debugprintf("maintqotd/subinfo_opensource: a=%s\n",ap) ;
#endif

	if ((si = matostr(sources,3,sp,sl)) >= 0) {
	    switch (si) {
	    case source_prog:
	        rs = subinfo_opensourceprog(sip,qf,ap) ;
	        break ;
	    default:
	        rs = SR_NOENT ;
	        break ;
	    } /* end switch */
	    fd = rs ;
	} else
	    rs = SR_NOENT ;

#if	CF_DEBUGS && CF_OPENDEF
	if (rs == SR_NOENT) {
	    rs = opendef(sip) ;
	    fd = rs ;
	}
#endif

#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_opensource: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (subinfo_opensource) */


static int subinfo_opensourceprog(MAINTQOTD *sip,cchar *qf,cchar *ap)
{
	int		rs ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("maintqotd/subinfo_opensourceprog: ent\n") ;
	debugprintf("maintqotd/subinfo_opensourceprog: a=%s\n",ap) ;
#endif

	rs = maintqotd_prog(sip,qf,ap) ;
	fd = rs ;

#if	CF_DEBUGS
	if (rs >= 0)
	debugfmode("maintqotd/subinfo_opensourceprog","_prog()",fd) ;
	debugprintf("maintqotd/subinfo_opensourceprog: _prog() rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (subinfo_opensourceprog) */


static int subinfo_defprog(MAINTQOTD *sip,cchar *qfn)
{
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
	if ((rs = subinfo_id(sip)) >= 0) {
	    vecstr	path, *plp = &path ;
	    if ((rs = vecstr_start(plp,5,0)) >= 0) {
	        if ((rs = subinfo_addourpath(sip,plp)) >= 0) {
	            int		i ;
	            for (i = 0 ; defprogs[i] != NULL ; i += 1) {
	                cchar	*prog = defprogs[i] ;
	                if ((rs = subinfo_defproger(sip,plp,prog,qfn)) >= 0) {
			    fd = rs ;
			    break ;
		        } else if (isNotPresent(rs)) {
			    rs = SR_OK ;
			}
			if (rs < 0) break ;
	            } /* end for */
		    if ((rs >= 0) && (fd < 0)) rs = SR_NOENT ;
	        } /* end if (subinfo_addourpath) */
	        rs1 = vecstr_finish(plp) ;
	        if (rs >= 0) rs = rs1 ;
	        if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	    } /* end if (vecstr) */
	} /* end if (subinfo_id) */
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (subinfo_defprog) */


static int subinfo_defproger(MAINTQOTD *sip,vecstr *plp,cchar *prog,cchar *qfn)
{
	IDS		*idp = &sip->id ;
	int		rs ;
	int		fd = -1 ;
	char		rbuf[MAXPATHLEN+1] ;
	if ((rs = getprogpath(idp,plp,rbuf,prog,-1)) >= 0) {
	    const int	alen = MAXNAMELEN ;
	    const int	rl = rs ;
	    char	abuf[MAXNAMELEN+1] ;
	    if (rl == 0) rs = mkpath1(rbuf,prog) ;
	    if (rs >= 0) {
		int	cl ;
		cchar	*cp ;
	        if ((cl = sfbasename(prog,-1,&cp)) > 0) {
		    if ((rs = sncpy1w(abuf,alen,cp,cl)) >= 0) {
			const int	of = O_RDONLY ;
		        cchar		*av[2] ;
		        cchar		**ev = NULL ;
	                av[0] = abuf ;
		        av[1] = NULL ;
	                if ((rs = uc_openprog(rbuf,of,av,ev)) >= 0) {
			    const mode_t	om = 0664 ;
			    const int		qof = (O_CREAT|O_TRUNC|O_RDWR) ;
		            const int		pfd = rs ;
			    if ((rs = uc_open(qfn,qof,om)) >= 0) {
				fd = rs ;
				rs = uc_writedesc(fd,pfd,-1) ;
				if (rs < 0) {
				    u_close(fd) ;
				    fd = -1 ;
				}
			    } /* end if (uc_open) */
			    u_close(pfd) ;
		        } /* end if (uc_openprog) */
	            } /* end if (sncpy) */
	        } else
		    rs = SR_NOENT ;
	    } /* end if (mkpath) */
	} /* end if (getprogpath) */
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (subinfo_defproger) */


static int subinfo_addourpath(MAINTQOTD *sip,vecstr *plp)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*path = getenv(VARPATH) ;
	if (path != NULL) {
	    rs = vecstr_addpathclean(plp,path,-1) ;
	    c += rs ;
	}
	if (rs >= 0) {
	    rs = subinfo_addprbins(sip,plp) ;
	    c += rs ;
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_addourpath) */


static int subinfo_addprbins(MAINTQOTD *sip,vecstr *plp)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	cchar		*pr = sip->pr ;
	for (i = 0 ; (rs >= 0) && (prbins[i] != NULL) ; i += 1) {
	    cchar	*prbin = prbins[i] ;
	    rs = subinfo_addprbin(sip,plp,pr,prbin) ;
	    c += rs ;
	} /* end for */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_addprbins) */


static int subinfo_addprbin(MAINTQOTD *sip,vecstr *plp,cchar *pr,cchar *prbin)
{
	int		rs ;
	int		c = 0 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mkpath2(tbuf,pr,prbin)) >= 0) {
	    struct ustat	sb ;
	    const int		tl = rs ;
	    if ((rs = u_stat(tbuf,&sb)) >= 0) {
		if (S_ISDIR(sb.st_mode)) {
		    const int	am = (R_OK|X_OK) ;
		    if ((rs = sperm(&sip->id,&sb,am)) >= 0) {
			rs = vecstr_adduniq(plp,tbuf,tl) ;
			if (rs < INT_MAX) c += 1 ;
		    } else if (isNotPresent(rs)) {
			rs = SR_OK ;
		    }
		} /* end if (is-dir) */
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    }
	} /* end if (mkpath) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_addprbin) */


static int subinfo_id(MAINTQOTD *sip)
{
	int		rs = SR_OK ;
	if (! sip->open.id) {
	    sip->open.id = TRUE ;
	    rs = ids_load(&sip->id) ;
	}
	return rs ;
}
/* end subroutine (subinfo_id) */


static int subinfo_dircheck(MAINTQOTD *sip,cchar *dname)
{
	struct ustat	sb ;
	const mode_t	dm = (0777 | S_ISGID) ;
	const uid_t	euid = sip->euid ;
	const int	nrs = SR_NOENT ;
	int		rs ;

	if ((rs = u_stat(dname,&sb)) >= 0) {
	    if (sb.st_uid == euid) {
		rs = subinfo_dirminmode(sip,dname,dm) ;
	    }
	} else if (rs == nrs) {
	    if ((rs = mkdirs(dname,dm)) >= 0) {
		rs = subinfo_dirminmode(sip,dname,dm) ;
	    } /* end if (mkdirs) */
	} /* end if (stat) */

	return rs ;
}
/* end subroutine (subinfo_dircheck) */


static int subinfo_dirminmode(MAINTQOTD *sip,cchar *dname,mode_t dm)
{
	const uid_t	euid = sip->euid ;
	int		rs ;
	if ((rs = uc_minmod(dname,dm)) >= 0) {
	    if (sip->uid_pr != euid) {
		u_chown(dname,sip->uid_pr,sip->gid_pr) ;
	    }
	} /* end if (uc_minmod) */
	return rs ;
}
/* end subroutine (subinfo_dirminmode) */


static int config_start(CONFIG *csp,MAINTQOTD *sip,cchar *cfname)
{
	int		rs ;
	char		tmpfname[MAXPATHLEN+1] = { 0 } ;

	if (cfname == NULL) return SR_FAULT ;

	memset(csp,0,sizeof(struct config)) ;
	csp->sip = sip ;

	if ((rs = config_findfile(csp,tmpfname,cfname)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    if (rs > 0) cfname = tmpfname ;

#if	CF_DEBUGS
	    debugprintf("maintqotd/config_start: mid rs=%d cfname=%s\n",
		rs,cfname) ;
#endif

	    if ((rs = paramfile_open(&csp->p,envv,cfname)) >= 0) {
	        if ((rs = config_cookbegin(csp)) >= 0) {
	            csp->f_p = TRUE ;
	        }
	        if (rs < 0)
	            paramfile_close(&csp->p) ;
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} else if (isNotPresent(rs))
	    rs = SR_OK ;

	if (rs >= 0) csp->magic = MAINTQOTD_CONFMAGIC ;

#if	CF_DEBUGS
	debugprintf("maintqotd/config_start: ret rs=%d f=%u\n",rs,csp->f_p) ;
#endif

	return rs ;
}
/* end subroutine (config_start) */


static int config_findfile(CONFIG *csp,char tbuf[],cchar *cfname)
{
	MAINTQOTD	*sip = csp->sip ;
	VECSTR		sv ;
	int		rs ;
	int		pl = 0 ;

	tbuf[0] = '\0' ;
	if ((rs = vecstr_start(&sv,6,0)) >= 0) {
	    const int	tlen = MAXPATHLEN ;

	    vecstr_envset(&sv,"p",sip->pr,-1) ;
	    vecstr_envset(&sv,"e","etc",-1) ;
	    vecstr_envset(&sv,"n",sip->sn,-1) ;

	    rs = permsched(csched,&sv,tbuf,tlen,cfname,R_OK) ;
	    pl = rs ;

	    vecstr_finish(&sv) ;
	} /* end if (finding file) */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (config_findfile) */


static int config_cookbegin(CONFIG *csp)
{
	MAINTQOTD	*sip = csp->sip ;
	const int	hlen = MAXHOSTNAMELEN ;
	int		rs ;
	char		hbuf[MAXHOSTNAMELEN+1] ;

	if ((rs = expcook_start(&csp->cooks)) >= 0) {
	    int		i ;
	    int		kch ;
	    int		vl ;
	    cchar	*ks = "PSNDHRU" ;
	    cchar	*vp ;
	    char	kbuf[2] ;

	    kbuf[1] = '\0' ;
	    for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	        kch = MKCHAR(ks[i]) ;
	        vp = NULL ;
	        vl = -1 ;
	        switch (kch) {
	        case 'P':
	            vp = sip->pn ;
	            break ;
	        case 'S':
	            vp = sip->sn ;
	            break ;
	        case 'N':
	            vp = sip->nn ;
	            break ;
	        case 'D':
	            vp = sip->dn ;
	            break ;
	        case 'H':
	            {
	                cchar	*nn = sip->nn ;
	                cchar	*dn = sip->dn ;
	                rs = snsds(hbuf,hlen,nn,dn) ;
	                vl = rs ;
	                vp = hbuf ;
	            }
	            break ;
	        case 'R':
	            vp = sip->pr ;
	            break ;
	        case 'U':
	            vp = sip->un ;
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (vp != NULL)) {
	            kbuf[0] = kch ;
	            rs = expcook_add(&csp->cooks,kbuf,vp,vl) ;
	        }
	    } /* end for */

	    if (rs >= 0) {
	        if ((vl = sfbasename(sip->pr,-1,&vp)) > 0) {
	            rs = expcook_add(&csp->cooks,"RN",vp,vl) ;
	        }
	    }

	    if (rs >= 0) {
	        if ((rs = ctdeci(hbuf,hlen,sip->mjd)) >= 0) {
	            rs = expcook_add(&csp->cooks,"MJD",hbuf,rs) ;
	        }
	    }

	    if (rs >= 0) {
	        csp->f_cooks = TRUE ;
	    } else
	        expcook_finish(&csp->cooks) ;
	} /* end if (expcook_start) */

	return rs ;
}
/* end subroutine (config_cookbegin) */


static int config_cookend(CONFIG *csp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp->f_cooks) {
	    csp->f_cooks = FALSE ;
	    rs1 = expcook_finish(&csp->cooks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (config_cookend) */


static int config_finish(CONFIG *csp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != MAINTQOTD_CONFMAGIC) return SR_NOTOPEN ;

	if (csp->f_p) {

	    if (csp->f_cooks) {
	        rs1 = config_cookend(csp) ;
	        if (rs >= 0) rs = rs1 ;
	    }

	    rs1 = paramfile_close(&csp->p) ;
	    if (rs >= 0) rs = rs1 ;

	    csp->f_p = FALSE ;
	} /* end if */

	return rs ;
}
/* end subroutine (config_finish) */


#if	CF_CONFIGCHECK
static int config_check(CONFIG *csp)
{
	MAINTQOTD	*sip = csp->sip ;
	int		rs = SR_OK ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != MAINTQOTD_CONFMAGIC) return SR_NOTOPEN ;

	if (csp->f_p) {
	    time_t	dt = sip->dt ;
	    if ((rs = paramfile_check(&csp->p,dt)) > 0)
	        rs = config_read(csp) ;
	}

	return rs ;
}
/* end subroutine (config_check) */
#endif /* CF_CONFIGCHECK */


static int config_read(CONFIG *csp)
{
	MAINTQOTD	*sip = csp->sip ;
	int		rs = SR_OK ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != MAINTQOTD_CONFMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("maintqotd/config_read: ent f_p=%u\n",csp->f_p) ;
#endif

	sip = csp->sip ;
	if (sip == NULL) return SR_FAULT ;

	if (csp->f_p) {
	    const int	elen = EBUFLEN ;
	    char	*ebuf ;
	    if ((rs = uc_malloc((elen+1),&ebuf)) >= 0) {
		rs = config_reader(csp,ebuf,elen) ;
		uc_free(ebuf) ;
	    } /* end if (memory-allocation) */
	} /* end if (avtive) */

#if	CF_DEBUGS
	debugprintf("maintqotd/config_read: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_read) */


static int config_reader(CONFIG *csp,char *ebuf,int elen)
{
	MAINTQOTD	*sip = csp->sip ;
	PARAMFILE	*pfp = &csp->p ;
	PARAMFILE_CUR	cur ;
	const int	vlen = VBUFLEN ;
	int		rs = SR_OK ;
	int		i ;
	int		vl, el ;
	int		v ;
	int		ml ;
	int		c = 0 ;
	char		vbuf[VBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("maintqotd/config_reader: ent f_active=%u\n",
		csp->f_p) ;
#endif
	if (sip == NULL) return SR_FAULT ;

	if (csp->f_p) {
	    for (i = 0 ; cparams[i] != NULL ; i += 1) {
		cchar	*cparam = cparams[i] ;

#if	CF_DEBUGS
	        debugprintf("mqintqotd/config_read: cparam=%s\n",cparam) ;
#endif

	        if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {

	            while (rs >= 0) {
	                vl = paramfile_fetch(pfp,cparam,&cur,vbuf,vlen) ;
#if	CF_DEBUGS
	                debugprintf("mqintqotd/config_read: "
			"paramfile_fetch() rs=%d\n",vl) ;
#endif
	                if (vl == SR_NOTFOUND) break ;
	                rs = vl ;
	                if (rs < 0) break ;

#if	CF_DEBUGS
	                    debugprintf("mqintqotd/config_read: "
				"vbuf=>%t<\n",vbuf,vl) ;
#endif

	                ebuf[0] = '\0' ;
	                el = 0 ;
	                if (vl > 0) {
	                    el = expcook_exp(&csp->cooks,0,ebuf,elen,vbuf,vl) ;
	                    if (el >= 0) ebuf[el] = '\0' ;
	                }

#if	CF_DEBUGS
	                debugprintf("maintqotd/config_read: "
				"ebuf=>%t<\n",ebuf,el) ;
#endif

	                if (el > 0) {
	                    cchar	*sn = sip->sn ;
	                    char	tbuf[MAXPATHLEN + 1] ;

			    c += 1 ;
	                    switch (i) {

	                    case cparam_logsize:
	                        if (cfdecmfi(ebuf,el,&v) >= 0) {
	                            if (v >= 0) {
	                                switch (i) {
	                                case cparam_logsize:
	                                    sip->logsize = v ;
	                                    break ;
	                                } /* end switch */
	                            }
	                        } /* end if (valid number) */
	                        break ;

	                    case cparam_to:
	                        if (cfdecti(ebuf,el,&v) >= 0) {
	                            if (v >= 0) {
	                                sip->to = v ;
	                            }
	                        } /* end if (valid number) */
	                        break ;

	                    case cparam_logfile:
	                        if (! sip->final.lfname) {
	                            cchar *lfn = sip->lfname ;
	                            cchar	*tfn = tbuf ;
	                            sip->final.lfname = TRUE ;
	                            sip->have.lfname = TRUE ;
	                            ml = setfname(sip,tbuf,ebuf,el,TRUE,
	                                LOGCNAME,sn,"") ;
	                            if ((lfn == NULL) || 
	                                (strcmp(lfn,tfn) != 0)) {
	                                cchar	**vpp = &sip->lfname ;
	                                sip->changed.lfname = TRUE ;
	                                rs = subinfo_setentry(sip,vpp,tbuf,ml) ;
	                            }
	                        }
	                        break ;

	                    case cparam_spooldir:
	                        if (sip->spooldname == NULL) {
	                            rs = subinfo_spooldir(sip,ebuf,el) ;
	                        }
	                        break ;

	                    case cparam_hostname:
	                        if (sip->hostname == NULL) {
	                            rs = subinfo_hostname(sip,ebuf,el) ;
	                        }
	                        break ;

	                    case cparam_source:
	                        rs = subinfo_source(sip,ebuf,el) ;
	                        break ;

	                    } /* end switch */

	                } /* end if (got one) */

	            } /* end while (fetching) */

	            paramfile_curend(pfp,&cur) ;
	        } /* end if (cursor) */

	        if (rs < 0) break ;
	    } /* end for (parameters) */
	} /* end if (active) */

#if	CF_DEBUGS
	debugprintf("maintqotd/config_reader: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (config_reader) */


static int getdefmjd(time_t dt)
{
	TMTIME		ct ;
	int		rs ;
	if (dt == 0) dt = time(NULL) ;
	if ((rs = tmtime_localtime(&ct,dt)) >= 0) {
	    int	y = (ct.year + TM_YEAR_BASE) ;
	    int	m = ct.mon ;
	    int	d = ct.mday ;
	    rs = getmjd(y,m,d) ;
	}
	return rs ;
}
/* end subroutine (getdefmjd) */


static int mkqfname(char *rbuf,cchar *qdname,int mjd)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,qdname,-1) ;
	    i += rs ;
	}

	if ((rs >= 0) && (i > 0) && (rbuf[i-1] != '/')) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,i,'q') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_deci(rbuf,rlen,i,mjd) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkqfname) */


/* calculate a file name */
static int setfname(sip,fname,ebuf,el,f_def,dname,name,suf)
MAINTQOTD	*sip ;
char		fname[] ;
cchar		ebuf[] ;
cchar		dname[], name[], suf[] ;
int		el ;
int		f_def ;
{
	int		rs = SR_OK ;
	int		ml ;
	cchar		*np ;
	char		tmpname[MAXNAMELEN + 1] ;

	if ((f_def && (ebuf[0] == '\0')) ||
	    (strcmp(ebuf,"+") == 0)) {

	    np = name ;
	    if ((suf != NULL) && (suf[0] != '\0')) {
	        np = tmpname ;
	        mkfnamesuf1(tmpname,name,suf) ;
	    }

	    if (np[0] != '/') {
	        if ((dname != NULL) && (dname[0] != '\0')) {
	            rs = mkpath3(fname,sip->pr,dname,np) ;
	        } else
	            rs = mkpath2(fname,sip->pr,np) ;
	    } else
	        rs = mkpath1(fname,np) ;

	} else if (strcmp(ebuf,"-") == 0) {

	    fname[0] = '\0' ;

	} else if (ebuf[0] != '\0') {

	    np = ebuf ;
	    if (el >= 0) {
	        np = tmpname ;
	        ml = MIN(MAXPATHLEN,el) ;
	        strwcpy(tmpname,ebuf,ml) ;
	    }

	    if (ebuf[0] != '/') {
	        if (strchr(np,'/') != NULL) {
	            rs = mkpath2(fname,sip->pr,np) ;
	        } else {
	            if ((dname != NULL) && (dname[0] != '\0')) {
	                rs = mkpath3(fname,sip->pr,dname,np) ;
	            } else
	                rs = mkpath2(fname,sip->pr,np) ;
	        } /* end if */
	    } else
	        rs = mkpath1(fname,np) ;

	} /* end if */

	return rs ;
}
/* end subroutine (setfname) */


static int mkourname(char *rbuf,cchar *pr,cchar *inter,cchar *sp,int sl)
{
	int		rs = SR_OK ;

	if (strnchr(sp,sl,'/') != NULL) {
	    if (sp[0] != '/') {
	        rs = mkpath2w(rbuf,pr,sp,sl) ;
	    } else {
	        rs = mkpath1w(rbuf,sp,sl) ;
	    }
	} else {
	    rs = mkpath3w(rbuf,pr,inter,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (mkourname) */


#if	CF_DEBUGS && CF_OPENDEF
/* ARGSUSED */
static int opendef(MAINTQOTD *sip)
{
	int		rs ;
	int		pipes[2] ;
	int		fd = -1 ;
	if ((rs = uc_piper(pipes,3)) >= 0) {
	    int		wfd = pipes[0] ;
	    int		sl ;
	    cchar	*sp = "hello world!\n" ;
	    fd = pipes[1] ;
	    sl = strlen(sp) ;
	    rs = uc_writen(wfd,sp,sl) ;
	    u_close(wfd) ;
	}
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opendef) */
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
static int debugmode(cchar *ids,cchar *s,cchar *fname)
{
	struct ustat	sb ;
	int		rs ;
	if ((rs = u_stat(fname,&sb)) >= 0) {
	    char	mstr[100+1] ;
	    snfilemode(mstr,100,sb.st_mode) ;
	    debugprintf("%s: %s %s\n",ids,s,mstr) ;
	} else {
	    debugprintf("%s: %s rs=%d\n",ids,s,rs) ;
	}
	return rs ;
}
/* end subroutine (debugmode) */
#endif /* CF_DEBUGS */


#if	CF_DEBUGS
static int debugfmode(cchar *id,cchar *s,int fd)
{
	struct ustat	sb ;
	int		rs ;
		char	mstr[100+1] ;
		u_fstat(fd,&sb) ;
		snfilemode(mstr,100,sb.st_mode) ;
	rs = debugprintf("%s: %s m=%s\n",id,s,mstr) ;
	return rs ;
}
/* end subroutine (debugfmode) */
#endif /* CF_DEBUGS */


