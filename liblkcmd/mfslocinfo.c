/* mfs-locinfo */

/* MFS-locinfo (extra code) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_TMPGROUP	1		/* change-group on TMP directory */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */
#define	CF_DEBUGDUMP	0		/* debug-dump */


/* revision history:

	= 2011-01-25, David A­D­ Morano
        I had to separate this code due to AST-code conflicts over the system
        socket structure definitions.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2011,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is extra MFS code that was sepeated out from the main source due to
        AST-code conflicts (see notes elsewhere also).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<sys/msg.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<mkpath.h>
#include	<vecstr.h>
#include	<lfm.h>
#include	<utmpacc.h>
#include	<ugetpw.h>
#include	<getax.h>
#include	<char.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"mfsmain.h"
#include	"mfslocinfo.h"
#include	"mfslisten.h"
#include	"mfslog.h"
#include	"defs.h"
#include	"msflag.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	PBUFLEN
#define	PBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(5 * MAXPATHLEN)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern int	snsd(char *,int,cchar *,uint) ;
extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(cchar *,int) ;
extern int	mkquoted(char *,int,cchar *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	rmdirfiles(cchar *,cchar *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	chownsame(cchar *,cchar *) ;
extern int	getsystypenum(char *,char *,cchar *,cchar *) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	isNotPresent(int) ;

extern int	proginfo_rootname(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	locinfo_envdefs(LOCINFO *) ;

static int	locinfo_cookbegin(LOCINFO *) ;
static int	locinfo_cookload(LOCINFO *) ;
static int	locinfo_cookend(LOCINFO *) ;

static int	locinfo_svbegin(LOCINFO *) ;
static int	locinfo_svload(LOCINFO *) ;
static int	locinfo_svend(LOCINFO *) ;

static int	locinfo_cmdsbegin(LOCINFO *) ;
static int	locinfo_cmdsend(LOCINFO *) ;

static int	locinfo_pidlockbegin(LOCINFO *) ;
static int	locinfo_pidlockend(LOCINFO *) ;

static int	locinfo_tmplockbegin(LOCINFO *) ;
static int	locinfo_tmplockend(LOCINFO *) ;

static int	locinfo_genlockbegin(LOCINFO *,LFM *,cchar *) ;
static int	locinfo_genlockend(LOCINFO *,LFM *) ;
static int	locinfo_genlockdir(LOCINFO *,cchar *) ;
static int	locinfo_genlockprint(LOCINFO *,cchar *,LFM_CHECK *) ;
static int	locinfo_tmpourdname(LOCINFO *) ;
static int	locinfo_runas(LOCINFO *) ;
static int	locinfo_chids(LOCINFO *,cchar *) ;
static int	locinfo_cookargsload(LOCINFO *,cchar **) ;
static int	locinfo_maintourtmper(LOCINFO *,int) ;

#if	CF_DEBUGS && CF_DEBUGDUMP
static int	vecstr_dump(vecstr *,cchar *) ;
#endif


/* local variables */

static cchar	*scheds[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	NULL
} ;

static cchar	*envdefs[] = {
	"PWD",
	"USERNAME",
	"HOME",
	"HOST"
	"PATH",
	NULL
} ;

static cchar	*envbads[] = {
	"_",
	"_A0",
	"_EF",
	"A__z",
	"RANDOM",
	"SECONDS",
	NULL
} ;

static cchar	*svctypes[] = {
	"mfserve",
	"tcpmux",
	"finger",
	NULL
} ;

static cchar	*cooks[] = {
	"SYSNAME",	/* OS system-name */
	"RELEASE",	/* OS system-release */
	"VERSION",	/* OS system-version */
	"MACHINE",	/* machine-name */
	"PLATFORM",	/* pathform */
	"ARCHITECTURE",	/* machine-architecture */
	"NCPU",		/* number of machine CPUs */
	"HZ",		/* OS clock tics */
	"U",		/* current user username */
	"G",		/* current user groupname */
	"HOME",		/* current user home directory */
	"SHELL",	/* current user shell */
	"ORGANIZATION",	/* current user organization name */
	"GECOSNAME",	/* current user gecos-name */
	"REALNAME",	/* current user real-name */
	"NAME",		/* current user name */
	"TZ",		/* current user time-zone */
	"N",		/* nodename */
	"D",		/* INET domainname (for current user) */
	"H",		/* INET hostname */
	"R",		/* program-root */
	"RN",		/* program-root-name */
	"PRN",		/* program-root name */
	"OSTYPE",
	"OSNUM",
	"S",		/* searchname */
	"O",		/* organization */
	"OO",		/* organization w/ hyphens */
	"OC",		/* org-code */
	"V",
	"tmpdname",
	NULL
} ;

enum cooks {
	cook_sysname,
	cook_release,
	cook_version,
	cook_machine,
	cook_platform,
	cook_architecture,
	cook_ncpu,
	cook_hz,
	cook_u,
	cook_g,
	cook_home,
	cook_shell,
	cook_organization,
	cook_gecosname,
	cook_realname,
	cook_name,
	cook_tz,
	cook_n,
	cook_d,
	cook_h,
	cook_r,
	cook_rn,
	cook_prn,
	cook_ostype,
	cook_osnum,
	cook_s,
	cook_o,
	cook_oo,
	cook_oc,
	cook_v,
	cook_tmpdname,
	cook_overlast
} ;

static cchar	*tmpleads[] = {
	"client",
	"sreqdb",
	"err",
	NULL
} ;


/* exported subroutines */


int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->uid_rootname = -1 ;
	lip->gid_rootname = -1 ;
	lip->intconf = TO_CONFIG ;
	lip->intdirmaint = TO_DIRMAINT ;
	lip->intclient = TO_DIRCLIENT ;
	lip->rfd = -1 ;
	lip->svctype = -1 ;
	lip->ti_tmpmaint = pip->daytime ;
	lip->ti_marklog = pip->daytime ;
	lip->ti_start = pip->daytime ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_start: ret rs=%d\n",rs) ;
#endif

	lip->f.adj = TRUE ;
	return rs ;
}
/* end subroutine (locinfo_start) */


int locinfo_finish(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUGS && CF_DEBUGDUMP
	if (lip->open.stores) {
	    vecstr_dump(&lip->stores,"finish") ;
	}
#endif

	rs1 = locinfo_cmdsend(lip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = locinfo_nsend(lip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = locinfo_lockend(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = locinfo_cookend(lip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_finish) */


int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	vecstr		*slp ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	slp = &lip->stores ;
	if (! lip->open.stores) {
	    rs = vecstr_start(slp,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
	        oi = vecstr_findaddr(slp,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(slp,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(slp,oi) ;
	    }
	} /* end if */

#if	CF_DEBUGS && CF_DEBUGDUMP
	vecstr_dump(slp,"after") ;
#endif
#if	CF_DEBUGS 
	debugprintf("locinfo_setentry: ret rs=%d l=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */


int locinfo_defs(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->open.svars) {
	    vecstr	*svp = &lip->svars ;
	    const int	tlen = MAXPATHLEN ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs >= 0) && (lip->svcfname == NULL)) {
	        cchar	*fn = SVCCNAME ;
	        if ((rs = permsched(scheds,svp,tbuf,tlen,fn,R_OK)) >= 0) {
	            cchar	**vpp = &lip->svcfname ;
	            rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (svcfname) */
	    if ((rs >= 0) && (lip->accfname == NULL)) {
	        cchar	*fn = ACCCNAME ;
	        if ((rs = permsched(scheds,svp,tbuf,tlen,fn,R_OK)) >= 0) {
	            cchar	**vpp = &lip->accfname ;
	            rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    }
	} else {
	    rs = SR_BUGCHECK ;
	}

	if (lip->msnode == NULL) {
	    lip->msnode = pip->nodename ;
	}

	return rs ;
}
/* end subroutine (locinfo_defs) */


int locinfo_defreg(LOCINFO *lip)
{

	if (lip->intspeed == 0) lip->intspeed = TO_SPEED ;

	return SR_OK ;
}
/* end subroutine (locinfo_defreg) */


int locinfo_defdaemon(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;

	if (lip->intspeed == 0) lip->intspeed = TO_SPEED ;

	if (pip->uid != pip->euid) {
	    u_setreuid(pip->euid,-1) ;
	}

	if (pip->gid != pip->egid) {
	    u_setregid(pip->egid,-1) ;
	}

	return SR_OK ;
}
/* end subroutine (locinfo_defdaemon) */


int locinfo_lockbegin(LOCINFO *lip)
{
	int		rs ;
	if ((rs = locinfo_pidlockbegin(lip)) >= 0) {
	    if ((rs = locinfo_tmplockbegin(lip)) >= 0) {
	        lip->open.locking = TRUE ;
	    }
	    if (rs < 0)
	        locinfo_pidlockend(lip) ;
	}
#if	CF_DEBUG
	{
	    PROGINFO	*pip = lip->pip ;
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_lockbegin: ret rs=%d\n",rs) ;
	}
#endif
	return rs ;
}
/* end subroutine (locinfo_lockbegin) */


int locinfo_lockcheck(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->open.locking) {
	    LFM_CHECK	ci ;

	    if ((rs >= 0) && lip->open.pidlock) {
	        rs = lfm_check(&lip->pidlock,&ci,pip->daytime) ;
	        if (rs == SR_LOCKLOST) {
	            locinfo_genlockprint(lip,pip->pidfname,&ci) ;
	        }
	    } /* end if (pidlock) */

	    if ((rs >= 0) && lip->open.tmplock) {
	        rs = lfm_check(&lip->tmplock,&ci,pip->daytime) ;
	        if (rs == SR_LOCKLOST) {
	            locinfo_genlockprint(lip,lip->tmpfname,&ci) ;
	        }
	    } /* end if (tmplock) */

	} /* end if (locking) */

	return rs ;
}
/* end subroutine (locinfo_lockcheck) */


int locinfo_lockend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.locking) {

	    rs1 = locinfo_tmplockend(lip) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = locinfo_pidlockend(lip) ;
	    if (rs >= 0) rs = rs1 ;

	    lip->open.locking = FALSE ;
	}  /* end if (locking) */

	return rs ;
}
/* end subroutine (locinfo_lockend) */


int locinfo_tmpourdir(LOCINFO *lip)
{
	int		rs ;
	int		pl = 0 ;
	if ((rs = locinfo_tmpourdname(lip)) >= 0) {
	    pl = rs ;
	    if ((rs = locinfo_runas(lip)) >= 0) {
	        USTAT		usb ;
	        const mode_t	dm = (lip->f.runasprn) ? 0775 : 0777 ;
	        cchar		*ourtmp = lip->tmpourdname ;
	        if ((rs = u_stat(ourtmp,&usb)) >= 0) {
	            if (S_ISDIR(usb.st_mode)) {
	                const int	am = (R_OK|W_OK|X_OK) ;
	                rs = u_access(ourtmp,am) ;
	            } else {
	                rs = SR_NOTDIR ;
	            }
	        } else if (isNotPresent(rs)) {
	            if ((rs = mkdirs(ourtmp,dm)) >= 0) {
	                if ((rs = uc_minmod(ourtmp,dm)) >= 0) {
	                    rs = locinfo_chids(lip,ourtmp) ;
	                }
	            }
	        }
	    } /* end if (locinfo_runas) */
	} /* end if (locinfo_tmpourdname) */
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (locinfo_tmpourdir) */


int locinfo_builtdname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		pl = 0 ;
	if (lip->builtdname == NULL) {
	    cchar	*pr = pip->pr ;
	    cchar	*sn = pip->searchname ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath4(tbuf,pr,"lib",sn,"svcs")) >= 0) {
	        const int	am = (R_OK|X_OK) ;
	        pl = rs ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("mfslocinfo_builtdname: tb=%s\n",tbuf) ;
#endif
	        if ((rs = uc_access(tbuf,am)) >= 0) {
	            cchar	**vpp = &lip->builtdname ;
	            rs = locinfo_setentry(lip,vpp,tbuf,pl) ;
	        } else if (isNotPresent(rs)) {
	            pl = 0 ;
	            rs = SR_OK ;
	        }
	    }
	} else {
	    pl = strlen(lip->builtdname) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("mfslocinfo_builtdname: dn=%s\n",lip->builtdname) ;
	    debugprintf("mfslocinfo_builtdname: ret rs=%d pl=%u\n",rs,pl) ;
	}
#endif
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (locinfo_builtdname) */


int locinfo_msfile(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const mode_t	msmode = 0666 ;
	int		rs = SR_OK ;
	int		mfl = -1 ;
	int		f_changed = FALSE ;
	cchar		*mfp ;
	char		tmpfname[MAXPATHLEN+1] ;

	mfp = lip->msfname ;
	if ((rs >= 0) && ((mfp == NULL) || (mfp[0] == '\0'))) {
	    f_changed = TRUE ;
	    mfp = tmpfname ;
	    rs = mkpath3(tmpfname,pip->pr,VCNAME,MSFNAME) ;
	    mfl = rs ;
	} /* end if (creating a default MS-file-name) */

	if ((rs >= 0) && (mfp != NULL) && f_changed) {
	    cchar	**vpp = &lip->msfname ;
	    lip->have.msfname = TRUE ;
	    rs = locinfo_setentry(lip,vpp,mfp,mfl) ;
	}

	if (rs >= 0) {
	    const int	am = (R_OK|W_OK) ;
	    rs = perm(lip->msfname,-1,-1,NULL,am) ;
	}

	if (rs == SR_ACCESS) {
	    rs = u_unlink(lip->msfname) ;
	    if (rs >= 0) rs = SR_NOENT ;
	}

	if (rs == SR_NOENT) {
	    const uid_t	uid = getuid() ;
	    const uid_t	euid = geteuid() ;
	    if ((rs = u_creat(lip->msfname,msmode)) >= 0) {
	        const int	fd = rs ;
	        if ((rs = uc_fminmod(fd,msmode)) >= 0) {
	            if (uid == euid) { /* we are not running SUID */
	                if ((rs = locinfo_rootids(lip)) >= 0) {
	                    const uid_t	uid_rn = lip->uid_rootname ;
	                    const gid_t	gid_rn = lip->gid_rootname ;
	                    u_fchown(fd,uid_rn,gid_rn) ; /* can fail */
	                } /* end if (root-ids) */
	            } /* end if (not SUID) */
	        } /* end if (uc_fminmod) */
	        u_close(fd) ;
	    } /* end if (create-file) */
	} /* end if (MS-file did not exist) */

	if (rs >= 0) {
	    if (pip->debuglevel > 0) {
	        shio_printf(pip->efp,
	            "%s: mf=%s\n",pip->progname,lip->msfname) ;
	    }
	    if (pip->open.logprog) {
	        logprintf(pip,"ms=%s",lip->msfname) ;
	    }
	}

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (locinfo_msfile) */


int locinfo_reqfname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		pl = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_reqfname: ent req=%s\n",lip->reqfname) ;
#endif

	if (lip->reqfname == NULL) {
	    if ((rs = locinfo_tmpourdir(lip)) >= 0) {
	        cchar	*tmpdname = lip->tmpourdname ;
	        cchar	*reqcname = REQCNAME ;
	        char	reqfname[MAXPATHLEN + 1] ;
	        if ((rs = mkpath2(reqfname,tmpdname,reqcname)) >= 0) {
	            cchar **vpp = &lip->reqfname ;
	            pl = rs ;
	            rs = locinfo_setentry(lip,vpp,reqfname,pl) ;
	        }
	    } /* end if */
	} else {
	    pl = strlen(lip->reqfname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("locinfo_reqfname: ret req=%s\n",lip->reqfname) ;
	    debugprintf("locinfo_reqfname: ret rs=%d pl=%u\n",rs,pl) ;
	}
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (locinfo_reqfname) */


int locinfo_rootids(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->gid_rootname < 0) {
	    if ((rs = proginfo_rootname(pip)) >= 0) {
	        struct passwd	pw ;
	        const int	pwlen = getbufsize(getbufsize_pw) ;
	        char		*pwbuf ;
	        if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	            cchar	*rn = pip->rootname ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("locinfo_rootids: rn=%s\n",rn) ;
#endif
	            if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,rn)) >= 0) {
	                lip->uid_rootname = pw.pw_uid ;
	                lip->gid_rootname = pw.pw_gid ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("locinfo_rootids: u=%d g=%d\n",
	                        pw.pw_uid,pw.pw_gid) ;
#endif
	            } /* end if (getpw_name) */
	            rs1 = uc_free(pwbuf) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (ma-a) */
	    } /* end if (rootname) */
	} /* end if (needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_rootids: ret rs=%d gid=%d\n",
	        rs,lip->gid_rootname) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rootids) */


int locinfo_nsbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_nsbegin: ent\n") ;
#endif
	if (! lip->open.ns) {
	    MFSNS	*nop = &lip->ns ;
	    cchar	*pr = pip->pr ;
	    if ((rs = mfsns_open(nop,pr)) >= 0) {
	        const int	no = MFSNS_ONOSERV ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("locinfo_nsbegin: mfsns_open() rs=%d\n",rs) ;
#endif
	        if ((rs = mfsns_setopts(nop,no)) >= 0) {
	            lip->open.ns = TRUE ;
	        }
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("locinfo_nsbegin: mfsns_setopts() rs=%d\n",rs) ;
#endif
	        if (rs < 0) {
	            lip->open.ns = FALSE ;
	            mfsns_close(nop) ;
	        }
	    } /* end if (mfsns_open) */
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_nsbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_nsbegin) */


int locinfo_nsend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.ns) {
	    MFSNS	*nop = &lip->ns ;
	    lip->open.ns = FALSE ;
	    rs1 = mfsns_close(nop) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_nsend) */


int locinfo_nslook(LOCINFO *lip,char *rbuf,int rlen,cchar *un,int w)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_nslook: ent un=%s w=%u\n",un,w) ;
#endif
	if ((rs = locinfo_nsbegin(lip)) >= 0) {
	    MFSNS	*nop = &lip->ns ;
	    rs = mfsns_get(nop,rbuf,rlen,un,w) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_nslook: mfsns_get() rs=%d\n",rs) ;
#endif
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("locinfo_nslook: ret rs=%d\n",rs) ;
	    debugprintf("locinfo_nslook: r=%t\n",rbuf,rs) ;
	}
#endif
	return rs ;
}
/* end subroutine (locinfo_nslook) */


int locinfo_maintourtmp(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const int	to = lip->intdirmaint ;
	int		rs = SR_OK ;
	int		f ;
	f = ((to > 0) && ((pip->daytime - lip->ti_tmpmaint) >= to)) ;
	if (f || lip->f.maint) {
	    lip->ti_tmpmaint = pip->daytime ;
	    if (lip->tmpourdname != NULL) {
	        const int	to_client = lip->intclient ;
	        if (to_client > 0) {
	            if ((rs = locinfo_maintourtmper(lip,to_client)) >= 0) {
	                char	tbuf[TIMEBUFLEN+1] ;
	                timestr_logz(pip->daytime,tbuf) ;
	                logprintf(pip,"%s maint-tmp",tbuf) ;
	                if (pip->debuglevel > 0) {
	                    cchar	*pn = pip->progname ;
	                    cchar	*fmt = "%s: maint-tmp (%d)\n" ;
	                    shio_printf(pip->efp,fmt,pn,rs) ;
	                }
	            } /* end if (locinfo_maintourtmper) */
	        } /* end if (positive) */
	    } /* end if (directory exists?) */
	} /* end if (tmp-dir maintenance needed) */
	return rs ;
}
/* end subroutine (locinfo_maintourtmp) */


int locinfo_cmdsload(LOCINFO *lip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (sl < 0) sl = strlen(sp) ;
	if (sl > 0) {
	    lip->f.cmds = TRUE ;
	    if ((rs = locinfo_cmdsbegin(lip)) >= 0) {
	        KEYOPT	*kop = &lip->cmds ;
	        rs = keyopt_loads(kop,sp,sl) ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_cmdsload) */


int locinfo_cmdscount(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (lip->open.cmds) {
	    KEYOPT	*kop = &lip->cmds ;
	    rs = keyopt_count(kop) ;
	}
	return rs ;
}
/* end subroutine (locinfo_cmdscount) */


/* Request-An-Exit */
int locinfo_reqexit(LOCINFO *lip,cchar *reason)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	cchar		*fmt = "reqexit · reason=%s" ;
	if (reason == NULL) reason = "" ;
	rs = logprintf(pip,fmt,reason) ;
	lip->f.reqexit = TRUE ;
	return rs ;
}
/* end subroutine (locinfo_reqexit) */


/* Is-Requested-Exit */
int locinfo_isreqexit(LOCINFO *lip)
{
	int		rs = MKBOOL(lip->f.reqexit) ;
	return rs ;
}
/* end subroutine (locinfo_isreqexit) */


/* Get-Accept-Timeout */
int locinfo_getaccto(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = lip->to_accept ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("locinfo_getaccto: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_getaccto) */


/* New-Serial */
int locinfo_newserial(LOCINFO *lip)
{
	int		s = lip->serial++ ;
	return s ;
}
/* end subroutine (locinfo_newserial) */


int locinfo_varbegin(LOCINFO *lip)
{
	int		rs ;
	if ((rs = locinfo_svbegin(lip)) >= 0) {
	    rs = locinfo_cookbegin(lip) ;
	    if (rs < 0)
	        locinfo_svend(lip) ;
	}
	return rs ;
}
/* end subroutine (locinfo_varbegin) */


int locinfo_varend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = locinfo_svend(lip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = locinfo_cookend(lip) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_varend) */


int locinfo_varsub(LOCINFO *lip,char *rbuf,int rlen,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		rl = 0 ;
	if (lip->open.subs) {
	    varsub	*vsp = &lip->subs ;
	    const int	vlen = VBUFLEN ;
	    char	vbuf[VBUFLEN+1] ;
	    if ((rs = varsub_expand(vsp,vbuf,vlen,sp,sl)) >= 0) {
	        EXPCOOK	*ecp = &lip->cooks ;
	        rs = expcook_exp(ecp,0,rbuf,rlen,vbuf,rs) ;
	        rl = rs ;
	    }
	}
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (locinfo_varsub) */


int locinfo_daemonbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	ENVHELP		*ehp = &lip->envs ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = envhelp_start(ehp,envbads,pip->envv)) >= 0) {
	    VARSUB	*slp = &lip->subs ;
	    if ((rs = locinfo_envdefs(lip)) >= 0) {
	        const int	n = 250 ;
	        lip->open.envs = TRUE ;
	        if ((rs = varsub_start(slp,n)) >= 0) {
	            cchar	**ev ;
	            if ((rs = envhelp_getvec(ehp,&ev)) >= 0) {
	                if ((rs = varsub_addvaquick(slp,ev)) >= 0) {
	                    lip->open.subs = TRUE ;
	                }
	            }
	            if (rs < 0) {
	                varsub_finish(slp) ;
	            }
	        } /* end if (varsub_start) */
	    } /* end if (locinfo_envdefs) */
	    if (rs < 0) {
	        lip->open.envs = FALSE ;
	        envhelp_finish(ehp) ;
	    }
	} /* end if (envhelp_start) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("locinfo_daemonbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_daemonbegin) */


int locinfo_daemonend(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip == NULL) return SR_FAULT ;
	if (lip->open.subs) {
	    VARSUB	*slp = &lip->subs ;
	    rs1 = varsub_finish(slp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	if (lip->open.envs) {
	    ENVHELP	*ehp = &lip->envs ;
	    rs1 = envhelp_finish(ehp) ;
	    if (rs >= 0) rs = rs1 ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("locinfo_daemonend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_daemonend) */


/* load some dynamic service values into cookie keys */
int locinfo_cooksvc(LOCINFO *lip,cchar *svc,cchar *ss,cchar **sav,int f_long)
{
	PROGINFO	*pip = lip->pip ;
	EXPCOOK		*ecp = &lip->cooks ;
	int		rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("locinfo_cooksvc: ent\n") ;
	    debugprintf("locinfo_cooksvc: s=%s ss=%s\n",svc,ss) ;
	}
#endif
	if (pip == NULL) return SR_FAULT ;
	if ((rs = expcook_add(ecp,"s",svc,-1)) >= 0) {
	    if ((rs = expcook_add(ecp,"ss",ss,-1)) >= 0) {
	        cchar	*vp = (f_long) ? "1" : "0" ;
	        if ((rs = expcook_add(ecp,"w",vp,1)) >= 0) {
	            vp = (f_long) ? "2" : "1" ;
	            if ((rs = expcook_add(ecp,"ww",vp,1)) >= 0) {
	                rs = locinfo_cookargsload(lip,sav) ;
	            }
	        }
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("locinfo_cooksvc: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_cooksvc) */


int locinfo_svctype(LOCINFO *lip,cchar *ebuf,int el)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("locinfo_svctype: ent w=>%t<\n",ebuf,el) ;
	    debugprintf("locinfo_svctype: svctype=%d\n",lip->svctype) ;
	}
#endif
	if (pip == NULL) return SR_FAULT ;
	if (lip->svctype < 0) {
	    int		i ;
	    if ((i = matostr(svctypes,1,ebuf,el)) >= 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("locinfo_svctype: i=%u\n",i) ;
#endif
	        lip->svctype = i ;
	    } else {
	        rs = SR_INVALID ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_svctype) */


int locinfo_newreq(LOCINFO *lip,int n)
{
	lip->nreqs += n ;
	return SR_OK ;
}
/* end subroutine (locinfo_newreq) */


int locinfo_getreqs(LOCINFO *lip)
{
	return lip->nreqs ;
}
/* end subroutine (locinfo_getreqs) */


/* private subroutines */


static int locinfo_envdefs(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	ENVHELP		*ehp = &lip->envs ;
	const int	rsn = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		i ;
	for (i = 0 ; envdefs[i] != NULL ; i += 1) {
	    cchar	*var = envdefs[i] ;
	    if ((rs = envhelp_present(ehp,var,-1,NULL)) == rsn) {
	        const int	kc = MKCHAR(var[0]) ;
	        int		vl = -1 ;
	        cchar		*vp = NULL ;
	        switch (kc) {
	        case 'U':
	            vp = pip->username ;
	            break ;
	        case 'H':
	            if (strcmp(var,VARHOME) == 0) {
	                vp = pip->homedname ;
	            } else { /* HOST */
	                const int	hlen = MAXHOSTNAMELEN ;
	                cchar		*nn = pip->nodename ;
	                cchar		*dn = pip->domainname ;
	                char		hbuf[MAXHOSTNAMELEN+1] ;
	                if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	                    rs = envhelp_envset(ehp,var,hbuf,rs) ;
	                }
	            }
	            break ;
	        case 'P':
	            if (var[1] == 'A') { /* PATH */
	                const int	plen = (2*MAXPATHLEN) ;
	                char		*pbuf ;
	                if ((rs = uc_malloc((plen+1),&pbuf)) >= 0) {
	                    if ((rs = uc_confstr(_CS_PATH,pbuf,plen)) >= 0) {
	                        rs = envhelp_envset(ehp,var,pbuf,rs) ;
	                    } /* end if */
	                    uc_free(pbuf) ;
	                } /* end if (m-a-f) */
	            } else { /* PWD */
	                if ((rs = proginfo_pwd(pip)) >= 0) {
	                    vp = pip->pwd ;
	                    vl = rs ;
	                }
	            }
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (vp != NULL)) {
	            rs = envhelp_envset(ehp,var,vp,vl) ;
	        }
	    } /* end if (envhelp_present) */
	    if (rs < 0) break ;
	} /* end for */
	return rs ;
}
/* end subroutine (locinfo_envdefs) */


static int locinfo_cookbegin(LOCINFO *lip)
{
	EXPCOOK		*ecp = &lip->cooks ;
	int		rs ;
	if ((rs = expcook_start(ecp)) >= 0) {
	    lip->open.cooks = TRUE ;
	    rs = locinfo_cookload(lip) ;
	    if (rs < 0) {
	        lip->open.cooks = FALSE ;
	        expcook_finish(ecp) ;
	    }
	} /* end if */
	return rs ;
}
/* end subroutine (locinfo_cookbegin) */


static int locinfo_cookend(LOCINFO *lip)
{
	EXPCOOK		*ecp = &lip->cooks ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.cooks) {
	    lip->open.cooks = FALSE ;
	    rs1 = expcook_finish(ecp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_cookend) */


static int locinfo_cookload(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	EXPCOOK		*cop = &lip->cooks ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ci ;
	int		vl ;
	cchar		*vp ;
	char		tbuf[USERNAMELEN+1] = { 
	    0 	} ;
	char		nbuf[USERNAMELEN+1] = { 
	    0 	} ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_cookload: ent\n") ;
#endif

	for (ci = 0 ; cooks[ci] != NULL ; ci += 1) {
	    cchar	*k = cooks[ci] ;
	    vp = NULL ;
	    vl = -1 ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_cookload: k=%s\n",k) ;
#endif
	    switch (ci) {
	    case cook_sysname:
	        vp = pip->usysname ;
	        break ;
	    case cook_release:
	        vp = pip->urelease ;
	        break ;
	    case cook_version:
	        vp = pip->uversion ;
	        break ;
	    case cook_machine:
	        vp = pip->umachine ;
	        break ;
	    case cook_platform:
	        vp = pip->platform ;
	        break ;
	    case cook_architecture:
	        vp = pip->architecture ;
	        break ;
	    case cook_ncpu:
	        {
	            const int	dlen = DIGBUFLEN ;
	            char	dbuf[DIGBUFLEN + 1] ;
	            if (pip->ncpu >= 0) {
	                rs = ctdeci(dbuf,dlen,pip->ncpu) ;
	            } else {
	                strcpy(dbuf,"1") ;
	                rs1 = 1 ;
	            }
	            if (rs >= 0) {
	                rs = expcook_add(cop,k,dbuf,rs1) ;
	            }
	        } /* end block */
	        break ;
	    case cook_hz:
	        vp = pip->hz ;
	        break ;
	    case cook_u:
	        vp = pip->username ;
	        break ;
	    case cook_g:
	        vp = pip->groupname ;
	        break ;
	    case cook_home:
	        vp = pip->homedname ;
	        break ;
	    case cook_shell:
	        vp = pip->shell ;
	        break ;
	    case cook_organization:
	    case cook_o:
	        vp = pip->org ;
	        break ;
	    case cook_gecosname:
	        vp = pip->gecosname ;
	        break ;
	    case cook_realname:
	        vp = pip->realname ;
	        break ;
	    case cook_name:
	        vp = pip->name ;
	        break ;
	    case cook_tz:
	        vp = pip->tz ;
	        break ;
	    case cook_n:
	        vp = pip->nodename ;
	        break ;
	    case cook_d:
	        vp = pip->domainname ;
	        break ;
	    case cook_h:
	        {
	            const int	hnlen = MAXHOSTNAMELEN ;
	            cchar	*nn = pip->nodename ;
	            cchar	*dn = pip->domainname ;
	            char	hnbuf[MAXHOSTNAMELEN + 1] ;
	            if ((rs = snsds(hnbuf,hnlen,nn,dn)) >= 0) {
	                rs = expcook_add(cop,k,hnbuf,rs1) ;
	            }
	        } /* end block */
	        break ;
	    case cook_r:
	        vp = pip->pr ;
	        break ;
	    case cook_rn:
	    case cook_prn:
	        vp = pip->rootname ;
	        break ;
	    case cook_ostype:
	    case cook_osnum:
	        if (tbuf[0] == '\0') {
	            cchar	*sysname = pip->usysname ;
	            cchar	*release = pip->urelease ;
	            rs = getsystypenum(tbuf,nbuf,sysname,release) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("locinfo_cookload: getsystypenum() rs=%d\n",
	                    rs) ;
#endif
	        }
	        if (rs >= 0) {
	            switch (ci) {
	            case cook_ostype:
	                vp = tbuf ;
	                break ;
	            case cook_osnum:
	                vp = nbuf ;
	                break ;
	            } /* end switch */
	        } /* end if */
	        break ;
	    case cook_s:
	        vp = pip->searchname ;
	        break ;
	    case cook_oo:
	        {
	            const int	oolen = ORGLEN ;
	            int		i ;
	            int		ch ;
	            cchar	*o = pip->org ;
	            char	oobuf[ORGLEN + 1] ;
	            for (i = 0 ; (i < oolen) && *o ; i += 1) {
	                ch = MKCHAR(o[i]) ;
	                oobuf[i] = (char) ((CHAR_ISWHITE(ch)) ? '-' : ch) ;
	            }
	            oobuf[i] = '\0' ;
	            rs = expcook_add(cop,k,oobuf,i) ;
	        } /* end block */
	        break ;
	    case cook_oc:
	        vp = pip->orgcode ;
	        break ;
	    case cook_v:
	        vp = pip->version ;
	        break ;
	    case cook_tmpdname:
	        vp = pip->tmpdname ;
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("locinfo_cookload: v=%t\n",vp,vl) ;
#endif
	        rs = expcook_add(cop,k,vp,vl) ;
	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_cookload: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_cookload) */


int locinfo_svbegin(LOCINFO *lip)
{
	VECSTR		*svp = &lip->svars ;
	int		rs ;
	if ((rs = vecstr_start(svp,4,0)) >= 0) {
	    if ((rs = locinfo_svload(lip)) >= 0) {
	        lip->open.svars = TRUE ;
	    }
	    if (rs < 0) {
	        lip->open.svars = FALSE ;
	        vecstr_finish(svp) ;
	    }
	} /* end if */
	return rs ;
}
/* end subroutine (locinfo_svbegin) */


int locinfo_svend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.svars) {
	    VECSTR	*svp = &lip->svars ;
	    lip->open.svars = FALSE ;
	    rs1 = vecstr_finish(svp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_svend) */


static int locinfo_svload(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	vecstr		*svp = &lip->svars ;
	int		rs = SR_OK ;
	int		i ;
	int		vl ;
	cchar		keys[] = "pen" ;
	cchar		*vp ;
	char		ks[2] = { 
	    0, 0 	} ;
	for (i = 0 ; keys[i] != '\0' ; i += 1) {
	    const int	kch = MKCHAR(keys[i]) ;
	    vp = NULL ;
	    vl = -1 ;
	    switch (kch) {
	    case 'p':
	        vp = pip->pr ;
	        break ;
	    case 'e':
	        vp = "etc" ;
	        break ;
	    case 'n':
	        vp = pip->searchname ;
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
	        ks[0] = kch ;
	        rs = vecstr_envadd(svp,ks,vp,vl) ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	return rs ;
}
/* end subroutine (locinfo_svload) */


static int locinfo_cmdsbegin(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (! lip->open.cmds) {
	    KEYOPT	*kop = &lip->cmds ;
	    if ((rs = keyopt_start(kop)) >= 0) {
	        lip->open.cmds = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_cmdsbegin) */


static int locinfo_cmdsend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.cmds) {
	    KEYOPT	*kop = &lip->cmds ;
	    lip->open.cmds = FALSE ;
	    rs1 = keyopt_finish(kop) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_cmdsend) */


static int locinfo_pidlockbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	cchar		*lfn ;
	lfn = pip->pidfname ;
	if ((lfn != NULL) && (lfn[0] != '\0') && (lfn[0] != '-')) {
	    LFM		*lfp = &lip->pidlock ;
	    if (pip->debuglevel > 0) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: pidlock=%s\n" ;
	        shio_printf(pip->efp,fmt,pn,lfn) ;
	    }
	    if ((rs = locinfo_genlockbegin(lip,lfp,lfn)) > 0) {
	        lip->open.pidlock = TRUE ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_pidlockbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_pidlockbegin) */


static int locinfo_pidlockend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.pidlock) {
	    LFM		*lfp = &lip->pidlock ;
	    lip->open.pidlock = FALSE ;
	    rs1 = locinfo_genlockend(lip,lfp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_pidlockend) */


static int locinfo_tmplockbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	cchar		*lfn = lip->tmpfname ;
	if ((lfn == NULL) || (lfn[0] == '+')) {
	    if ((rs = locinfo_tmpourdir(lip)) >= 0) {
	        cchar	*tmpcname = "pid" ;
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(tbuf,lip->tmpourdname,tmpcname)) >= 0) {
	            cchar	**vpp = &lip->tmpfname ;
	            if ((rs = locinfo_setentry(lip,vpp,tbuf,rs)) >= 0) {
	                lip->open.tmpfname = TRUE ;
	                logprintf(pip,"tmp=%s",lip->tmpfname) ;
	            }
	        }
	    }
	} /* end if (null) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_tmplockbegin: mid2 rs=%d\n",rs) ;
#endif
	if (rs >= 0) {
	    lfn = lip->tmpfname ;
	    if ((lfn != NULL) && (lfn[0] != '-')) {
	        LFM	*lfp = &lip->tmplock ;
	        if (pip->debuglevel > 0) {
	            cchar	*pn = pip->progname ;
	            cchar	*fmt = "%s: tmplock=%s\n" ;
	            shio_printf(pip->efp,fmt,pn,lfn) ;
	        }
	        if ((rs = locinfo_genlockbegin(lip,lfp,lfn)) >= 0) {
	            lip->open.tmplock = TRUE ;
	        }
	    } /* end if (not null) */
	} /* end if (ok) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_tmplockbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_tmplockbegin) */


static int locinfo_tmplockend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.tmplock) {
	    LFM		*lfp = &lip->tmplock ;
	    lip->open.tmplock = FALSE ;
	    rs1 = locinfo_genlockend(lip,lfp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_tmplockend) */


static int locinfo_genlockbegin(LOCINFO *lip,LFM *lfp,cchar *lfn)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	int		f = FALSE ;
	if ((rs = locinfo_genlockdir(lip,lfn)) >= 0) {
	    LFM_CHECK	lc ;
	    const int	ltype = LFM_TRECORD ;
	    const int	to_lock = lip->to_lock ;
	    cchar	*nn = pip->nodename ;
	    cchar	*un = pip->username ;
	    cchar	*bn = pip->banner ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_genlockbegin: lfn=%s\n",lfn) ;
#endif
	    if ((rs = lfm_start(lfp,lfn,ltype,to_lock,&lc,nn,un,bn)) >= 0) {
	        f = TRUE ;
	    } else if ((rs == SR_LOCKLOST) || (rs == SR_AGAIN)) {
	        locinfo_genlockprint(lip,lfn,&lc) ;
	    }
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_genlockbegin: lfm_start() rs=%d\n",rs) ;
#endif
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_genlockbegin: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_genlockbegin) */


static int locinfo_genlockend(LOCINFO *lip,LFM *lfp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip == NULL) return SR_FAULT ;
	rs1 = lfm_finish(lfp) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (locinfo_genlockend) */


static int locinfo_genlockdir(LOCINFO *lip,cchar *lfn)
{
	int		rs = SR_OK ;
	if (lip == NULL) return SR_FAULT ;
	if ((lfn != NULL) && (lfn[0] != '\0') && (lfn[0] != '-')) {
	    int		cl ;
	    cchar	*cp ;
	    if ((cl = sfdirname(lfn,-1,&cp)) > 0) {
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = mkpath1w(tbuf,cp,cl)) >= 0) {
	            USTAT	usb ;
	            const int	rsn = SR_NOENT ;
	            if ((rs = u_stat(tbuf,&usb)) == rsn) {
	                const mode_t	dm = 0777 ;
	                rs = mkdirs(tbuf,dm) ;
	            } else if (rs >= 0) {
	                if (! S_ISDIR(usb.st_mode)) rs = SR_NOTDIR ;
	            }
	        } /* end if (mkpath) */
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_genlockdir) */


static int locinfo_genlockprint(LOCINFO *lip,cchar *lfn,LFM_CHECK *lcp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	cchar		*np ;
	char		timebuf[TIMEBUFLEN + 1] ;

	switch (lcp->stat) {
	case SR_AGAIN:
	    np = "busy" ;
	    break ;
	case SR_LOCKLOST:
	    np = "lost" ;
	    break ;
	default:
	    np = "unknown" ;
	    break ;
	} /* end switch */

	if (pip->open.logprog) {
	    loglock(pip,lcp,lfn,np) ;
	}

	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;

	    fmt = "%s: %s lock %s\n" ;
	    timestr_logz(pip->daytime,timebuf) ;
	    shio_printf(pip->efp,fmt,pn,timebuf,np) ;

	    fmt = "%s: other_pid=%d\n" ;
	    shio_printf(pip->efp,fmt,pn,lcp->pid) ;

	    if (lcp->nodename != NULL) {
	        fmt = "%s: other_node=%s\n" ;
	        shio_printf(pip->efp,fmt,pn,lcp->nodename) ;
	    }

	    if (lcp->username != NULL) {
	        fmt = "%s: other_user=%s\n" ;
	        rs = shio_printf(pip->efp,fmt,lcp->username) ;
	    }

	    if (lcp->banner != NULL) {
	        fmt = "%s: other_banner=>%s<\n" ;
	        shio_printf(pip->efp,fmt,lcp->banner) ;
	    }

	} /* end if (standard-error) */

	return rs ;
}
/* end subroutine (locinfo_genlockprint) */


static int locinfo_tmpourdname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		pl = 0 ;
	if (lip->tmpourdname == NULL) {
	    if ((rs = proginfo_rootname(pip)) >= 0) {
	        cchar	*tn = pip->tmpdname ;
	        cchar	*rn = pip->rootname ;
	        cchar	*sn = pip->searchname ;
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = mkpath3(tbuf,tn,rn,sn)) >= 0) {
	            cchar	**vpp = &lip->tmpourdname ;
	            pl = rs ;
	            rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
	        } /* end if (mkpath) */
	    } /* end if (proginfo_rootname) */
	} else {
	    pl = strlen(lip->tmpourdname) ;
	} /* end if (needed) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_tmpourdname: ret rs=%d pl=%u\n",rs,pl) ;
#endif
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (locinfo_tmpourdname) */


/* are we running with our username the same as our root name? */
static int locinfo_runas(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		f = lip->f.runasprn ;
	if (! lip->have.runasprn) {
	    PROGINFO	*pip = lip->pip ;
	    lip->have.runasprn = TRUE ;
	    if ((rs = proginfo_rootname(pip)) >= 0) {
	        cchar	*rn = pip->rootname ;
	        cchar	*un = pip->username ;
	        f = (strcmp(un,rn) == 0) ;
	        lip->f.runasprn = f ;
	    } /* end if (procinfo_rootname) */
	} /* end if (needed) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_runas) */


static int locinfo_chids(LOCINFO *lip,cchar *dname)
{
	int		rs ;
	if ((rs = locinfo_rootids(lip)) >= 0) {
	    const uid_t	uid = lip->uid_rootname ;
	    const gid_t	gid = lip->gid_rootname ;
	    if ((rs = locinfo_runas(lip)) > 0) {
	        rs = u_chown(dname,-1,gid) ;
	    } else if (rs == 0) {
	        rs = u_chown(dname,uid,gid) ;
	    }
	} /* end if (locinfo_rootids) */
	return rs ;
}
/* end subrutine (locinfo_chids) */


static int locinfo_cookargsload(LOCINFO *lip,cchar **sav)
{
	EXPCOOK		*ecp = &lip->cooks ;
	int		rs ;
	int		blen = 0 ;
	int		i ;
	char		*bbuf ;
	char		*bp ;

	for (i = 0 ; sav[i] != NULL ; i += 1) {
	    blen += ((2 * strlen(sav[i])) + 2 + 1) ;
	}

	if ((rs = uc_malloc((blen+1),&bbuf)) >= 0) {
	    bbuf[0] = '\0' ;
	    bp = bbuf ;
	    for (i = 0 ; (rs >= 0) && (sav[i] != NULL) ; i += 1) {
	        if (i > 0) {
	            *bp++ = ' ' ;
	            blen -= 1 ;
	        }
	        rs = mkquoted(bp,blen,sav[i],-1) ;
	        bp += rs ;
	        blen -= rs ;
	    } /* end for */
	    if (rs >= 0) {
	        rs = expcook_add(ecp,"a",bbuf,(bp-bbuf)) ;
	    }
	    uc_free(bbuf) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subrutine (locinfo_argsload) */


static int locinfo_maintourtmper(LOCINFO *lip,int to_client)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		i ;
	cchar		*dir = lip->tmpourdname ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_maintourtmp: dir=%s\n",dir) ;
#endif
	if (pip == NULL) return SR_FAULT ;
	for (i = 0 ; (rs >= 0) && (tmpleads[i] != NULL) ; i += 1) {
	    cchar	*pat = tmpleads[i] ;
	    rs = rmdirfiles(dir,pat,to_client) ;
	}
	return rs ;
}
/* end subroutine (locinfo_maintourtmper) */


#if	CF_DEBUGS && CF_DEBUGDUMP
static int vecstr_dump(vecstr *dlp,cchar *id)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	cchar		*cp ;
	for (i = 0 ; (rs1 = vecstr_get(dlp,i,&cp)) >= 0 ; i += 1) {
	    if (cp != NULL) {
	        debugprintf("mfslocinfo/%s: v%u{%p}=%s\n",id,i,cp,cp) ;
	    }
	} /* end for */
	if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	return rs ;
}
#endif /* CF_DEBUGS */


