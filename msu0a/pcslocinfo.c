/* pcs-locinfo */

/* PCS-locinfo (extra code) */
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

*/

/* Copyright © 2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is extra PCS code that was sepeated out from the main source due to
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
#include	<poll.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<vecstr.h>
#include	<lfm.h>
#include	<utmpacc.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"pcsmain.h"
#include	"pcslocinfo.h"
#include	"pcslog.h"
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
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	IPCBUFLEN
#define	IPCBUFLEN	MSGBUFLEN
#endif

#define	DEBUGFNAME	"/tmp/pcs.deb"

#ifndef	TTYFNAME
#define	TTYFNAME	"/dev/tty"
#endif


/* external subroutines */

extern int	snsd(char *,int,cchar *,uint) ;
extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(cchar *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	rmdirfiles(cchar *,cchar *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
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

#if	CF_DEBUGS && CF_DEBUGDUMP
static int vecstr_dump(vecstr *,cchar *) ;
#endif


/* local variables */


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
	lip->to_cache = -1 ;
	lip->to_lock = -1 ;
	lip->ti_marklog = pip->daytime ;
	lip->ti_start = pip->daytime ;

	lip->f.adj = TRUE ;
	return rs ;
}
/* end subroutine (locinfo_start) */


int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

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

#if	CF_DEBUGS && CF_DEBUGDUMP
	vecstr_dump(slp,"before") ;
#endif

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
	debugprintf("locinfo_setentry: ret rs=%d l=%u\n",rs,len) ;
	vecstr_dump(slp,"after") ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */


int locinfo_defs(LOCINFO *lip)
{

	if (lip->msnode == NULL) {
	    PROGINFO	*pip = lip->pip ;
	    lip->msnode = pip->nodename ;
	}

	return SR_OK ;
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

	if (pip->uid != pip->euid)
	    u_setreuid(pip->euid,-1) ;

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
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	int		pl = 0 ;
	if (pip == NULL) return SR_FAULT ;
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
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("pcslocinfo_tmpourdir: ret rs=%d pl=%u\n",rs,pl) ;
#endif
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (locinfo_tmpourdir) */


int locinfo_msfile(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const mode_t	msmode = 0666 ;
	int		rs = SR_OK ;
	int		mfl = -1 ;
	int		f_changed = FALSE ;
	cchar		*mfp ;
	char		tmpfname[MAXPATHLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_msfile: ent\n") ;
#endif

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
	            "%s: msf=%s\n",pip->progname,lip->msfname) ;
	    }
	    if (pip->open.logprog) {
	        logprintf(pip,"ms=%s", lip->msfname) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_msfile: ret rs=%d f_cng=%u\n",rs,f_changed) ;
#endif

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
	    if ((rs = locinfo_tmpourdname(lip)) >= 0) {
	        cchar	*reqcname = REQCNAME ;
	        char	reqfname[MAXPATHLEN + 1] ;
	        if ((rs = mkpath2(reqfname,lip->tmpourdname,reqcname)) >= 0) {
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
	debugprintf("locinfo_reqfname: ret rs=%d pl=%u\n",rs,pl) ;
	debugprintf("locinfo_reqfname: ret req=%s\n",lip->reqfname) ;
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
	            if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,rn)) >= 0) {
			lip->uid_rootname = pw.pw_uid ;
			lip->gid_rootname = pw.pw_gid ;
	            } /* end if (getpw_name) */
		    rs1 = uc_free(pwbuf) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (ma-a) */
	    } /* end if (rootname) */
	} /* end if (needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("pcslocinfo_rootids: ret rs=%d gid=%d\n",
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
	    PCSNS	*nop = &lip->ns ;
	    cchar	*pr = pip->pr ;
	    if ((rs = pcsns_open(nop,pr)) >= 0) {
	        const int	no = PCSNS_ONOSERV ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_nsbegin: pcsns_open() rs=%d\n",rs) ;
#endif
		if ((rs = pcsns_setopts(nop,no)) >= 0) {
		    lip->open.ns = TRUE ;
		}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_nsbegin: pcsns_setopts() rs=%d\n",rs) ;
#endif
		if (rs < 0) {
		    lip->open.ns = FALSE ;
		    pcsns_close(nop) ;
		}
	    } /* end if (pcsns_open) */
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
	    PCSNS	*nop = &lip->ns ;
	    lip->open.ns = FALSE ;
	    rs1 = pcsns_close(nop) ;
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
	    PCSNS	*nop = &lip->ns ;
	    rs = pcsns_get(nop,rbuf,rlen,un,w) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("locinfo_nslook: pcsns_get() rs=%d\n",rs) ;
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


int locinfo_dirmaint(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const int	to = lip->intdirmaint ;
	int		rs = SR_OK ;
	int		f ;
	f = ((to > 0) && ((pip->daytime - lip->ti_tmpmaint) >= to)) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	debugprintf("pcslocinfo_dirmaint: ent f=%u f_maint=%u\n",
		f,lip->f.maint) ;
	debugprintf("pcslocinfo_dirmaint: intclient=%u\n",lip->intclient) ;
	debugprintf("pcslocinfo_dirmaint: tmpourdname=%s\n",lip->tmpourdname) ;
	}
#endif
	if (f || lip->f.maint) {
	    lip->ti_tmpmaint = pip->daytime ;
	    if (lip->tmpourdname != NULL) {
		const int	to_client = lip->intclient ;
		if (to_client > 0) {
		    cchar	*dir = lip->tmpourdname ;
		    cchar	*pat = "client" ;
	            if ((rs = rmdirfiles(dir,pat,to_client)) >= 0) {
		        char	tbuf[TIMEBUFLEN+1] ;
		        timestr_logz(pip->daytime,tbuf) ;
		        logprintf(pip,"%s dirmaint (%d)",tbuf,rs) ;
			if (pip->debuglevel > 0) {
			    cchar	*pn = pip->progname ;
			    cchar	*fmt = "%s: maint-tmp (%d)\n" ;
			    shio_printf(pip->efp,fmt,pn,rs) ;
			}
		    }
		} /* end if (positive) */
	    } /* end if (directory exists?) */
	} /* end if (tmp-dir maintenance needed) */
	return rs ;
}
/* end subroutine (locinfo_dirmaint) */


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


int locinfo_isreqexit(LOCINFO *lip)
{
	int		rs = MKBOOL(lip->f.reqexit) ;
	return rs ;
}
/* end subroutine (locinfo_isreqexit) */


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
	return rs ;
}
/* end subroutine (locinfo_pidlockbegin) */


static int locinfo_pidlockend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.pidlock) {
	    LFM	*lfp = &lip->pidlock ;
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
	    if ((rs = locinfo_tmpourdname(lip)) >= 0) {
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
	    }
	}
	return rs ;
}
/* end subroutine (tmplockbegin) */


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
	int		rs ;
	int		f = FALSE ;
	if ((rs = locinfo_genlockdir(lip,lfn)) >= 0) {
	    PROGINFO	*pip = lip->pip ;
	    LFM_CHECK	lc ;
	    const int	ltype = LFM_TRECORD ;
	    const int	to_lock = lip->to_lock ;
	    cchar	*nn = pip->nodename ;
	    cchar	*un = pip->username ;
	    cchar	*bn = pip->banner ;
	    if ((rs = lfm_start(lfp,lfn,ltype,to_lock,&lc,nn,un,bn)) >= 0) {
	        f = TRUE ;
	    } else if ((rs == SR_LOCKLOST) || (rs == SR_AGAIN)) {
		locinfo_genlockprint(lip,lfn,&lc) ;
	    }
	}
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
	if (DEBUGLEVEL(5)) {
	    debugprintf("pcslocinfo_tmpourdname: tmpourdiname=%s\n",
		lip->tmpourdname) ;
	    debugprintf("pcslocinfo_tmpourdname: ret rs=%d pl=%u\n",rs,pl) ;
	}
#endif
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (locinfo_tmpourdname) */


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


#if	CF_DEBUGS && CF_DEBUGDUMP
static int vecstr_dump(vecstr *dlp,cchar *id)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	cchar		*cp ;
	for (i = 0 ; (rs1 = vecstr_get(dlp,i,&cp)) >= 0 ; i += 1) {
	    if (cp != NULL) {
		debugprintf("pcslocinfo/%s: v%u{%p}=%s\n",id,i,cp,cp) ;
	    }
	} /* end for */
	if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	return rs ;
}
#endif /* CF_DEBUGS */


