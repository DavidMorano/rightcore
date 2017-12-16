/* msu-locinfo */

/* MSU-locinfo (extra code) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_TMPGROUP	1		/* change-group on TMP directory */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2011-01-25, David A­D­ Morano
        I had to separate this code due to AST-code conflicts over the system
        socket structure definitions.

*/

/* Copyright © 2004,2005,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is extra MSU code that was sepeated out from the main source due to
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
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<vecstr.h>
#include	<lfm.h>
#include	<utmpacc.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<localmisc.h>

#include	"msumain.h"
#include	"msumainer.h"
#include	"msulocinfo.h"
#include	"msulog.h"
#include	"defs.h"
#include	"msflag.h"
#include	"shio.h"


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

#define	DEBUGFNAME	"/tmp/msu.deb"

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

static int locinfo_lockbeginone(LOCINFO *,LFM *,cchar *) ;


/* local variables */


/* exported subroutines */


int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->gid_rootname = -1 ;
	lip->intconfig = TO_CONFIG ;
	lip->to_cache = -1 ;
	lip->to_lock = -1 ;
	lip->ti_marklog = pip->daytime ;
	lip->ti_start = pip->daytime ;

#ifdef	COMMENT
	if (pip->uid != pip->euid)
	    u_setreuid(pip->uid,-1) ;

	if (pip->gid != pip->egid)
	    u_setregid(pip->gid,-1) ;
#endif /* COMMENT */

	{
	    time_t	t ;
	    rs = utmpacc_boottime(&t) ;
	    lip->ti_boot = t ;
	}

	lip->f.listen = TRUE ;

	return rs ;
}
/* end subroutine (locinfo_start) */


int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	memset(lip,0,sizeof(LOCINFO)) ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar vp[],int vl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;

	    if (*epp != NULL) oi = vecstr_findaddr(&lip->stores,*epp) ;

	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,len,epp) ;
	    } else
		*epp = NULL ;

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&lip->stores,oi) ;

	} /* end if */

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

	if (pip->gid != pip->egid)
	    u_setregid(pip->egid,-1) ;

	return SR_OK ;
}
/* end subroutine (locinfo_defdaemon) */


int locinfo_lockbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	LFM		*lfp ;
	int		rs = SR_OK ;
	cchar		*pidcname = "pid" ;
	cchar		*lockfname ;

	if (rs >= 0) {
	    lockfname = pip->pidfname ;
	    lfp = &lip->pidlock ;
	    rs = locinfo_lockbeginone(lip,lfp,lockfname) ;
	    lip->open.pidlock = (rs > 0) ;
	}

	if (rs >= 0) {
	    if (lip->tmpfname == NULL) {
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = locinfo_tmpourdname(lip)) >= 0) {
	            if ((rs = mkpath2(tbuf,lip->tmpourdname,pidcname)) >= 0) {
		        cchar	**vpp = &lip->tmpfname ;
		        rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
		    }
	        }
	    } /* end if (null) */
	    if (rs >= 0) {
		lockfname = lip->tmpfname ;
	        lfp = &lip->tmplock ;
	        rs = locinfo_lockbeginone(lip,lfp,lockfname) ;
	        lip->open.tmplock = (rs > 0) ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_lockbegin) */


int locinfo_lockcheck(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	LFM_CHECK	ci ;
	int		rs = SR_OK ;

	if ((rs >= 0) && lip->open.pidlock) {
	    rs = lfm_check(&lip->pidlock,&ci,pip->daytime) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("msulocinfo/_lockcheck: lfm_check() rs=%d\n",
	        rs) ;
	    if (rs < 0) {
	        debugprintf("msulocinfo/_lockcheck: pid=%d\n",ci.pid) ;
	        debugprintf("msulocinfo/_lockcheck: node=%s\n",
	            ci.nodename) ;
	        debugprintf("msulocinfo/_lockcheck: user=%s\n",
	            ci.username) ;
	        debugprintf("msulocinfo/_lockcheck: banner=%s\n",
	            ci.banner) ;
	    }
	}
#endif /* CF_DEBUG */

	    if (rs == SR_LOCKLOST) msumainlockprint(pip,pip->pidfname,&ci) ;

	} /* end if (pidlock) */

	if ((rs >= 0) && lip->open.tmplock) {
	    rs = lfm_check(&lip->tmplock,&ci,pip->daytime) ;
	    if (rs == SR_LOCKLOST) msumainlockprint(pip,lip->tmpfname,&ci) ;
	}

	return rs ;
}
/* end subroutine (locinfo_lockcheck) */


int locinfo_lockend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.tmplock) {
	    lip->open.tmplock = FALSE ;
	    rs1 = lfm_finish(&lip->tmplock) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.pidlock) {
	    lip->open.pidlock = FALSE ;
	    rs1 = lfm_finish(&lip->pidlock) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_lockend) */


int locinfo_tmpourdname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	mode_t		dm = 0775 ;
	int		rs = SR_OK ;
	int		pl = 0 ;

	if (lip->tmpourdname == NULL) {
	    int		f_runasprn ;
	    int		f_created = FALSE ;
	    cchar	*sn = pip->searchname ;
	    cchar	*tn = pip->tmpdname ;
	    cchar	*rn = NULL ;
	    char	tmpourdname[MAXPATHLEN + 1] = { 0 } ;
	    if (pip->rootname == NULL) {
	        rs = proginfo_rootname(pip) ;
	    }
	    rn = pip->rootname ;
	    f_runasprn = (strcmp(pip->username,rn) == 0) ;
	    if (! f_runasprn) dm = 0777 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("msulocinfo_tmpourdname: f_runasprn=%u\n",
		f_runasprn) ;
#endif

/* create the tmp-dir as needed */

	    if (rs >= 0) {
	        if ((rs = mkpath3(tmpourdname,tn,rn,sn)) >= 0) {
		    struct ustat	sb ;
	            pl = rs ;
	            if ((rs = u_stat(tmpourdname,&sb)) >= 0) {
	    		int		f_needmode = FALSE ;
	                if (S_ISDIR(sb.st_mode)) {
			    const int	am = (R_OK|W_OK|X_OK) ;
	                    f_needmode = ((sb.st_mode & dm) != dm) ;
			    if ((rs = u_access(tmpourdname,am)) >= 0) {
				if (f_needmode && (pip->euid == sb.st_uid)) {
	    		    	    rs = uc_minmod(tmpourdname,dm) ;
				}
			    }
	                } else {
	                    rs = SR_NOTDIR ;
		        }
	            } else if (isNotPresent(rs)) {
		        f_created = TRUE ;
	                if ((rs = mkdirs(tmpourdname,dm)) >= 0) {
	    		    rs = uc_minmod(tmpourdname,dm) ;
			}
		    }
    
#if	CF_TMPGROUP
	if ((rs >= 0) && f_created) {
	    if ((rs = locinfo_gidrootname(lip)) >= 0) {
		const uid_t	uid = lip->uid_rootname ;
		const gid_t	gid = lip->gid_rootname ;
		if (f_runasprn) {
	            rs = u_chown(tmpourdname,-1,gid) ;
		} else {
	            rs = u_chown(tmpourdname,uid,gid) ;
		}
	    } /* end if (ok) */
	}
#endif /* CF_TMPGROUP */

	if (rs >= 0) {
	    cchar	**vpp = &lip->tmpourdname ;
	    rs = locinfo_setentry(lip,vpp,tmpourdname,pl) ;
	}

		} /* end if (mkpath) */
	    } /* end if (ok) */
	} else {
	    pl = strlen(lip->tmpourdname) ;
	}

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (locinfo_tmpourdname) */


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
		    if ((rs = proginfo_rootname(pip)) >= 0) {
	                struct passwd	pw ;
			const int	pwlen = getbufsize(getbufsize_pw) ;
	                char		*pwbuf ;
			if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
			    cchar	*rn = pip->rootname ;
	                    if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,rn)) >= 0) {
				const uid_t	uid = pw.pw_uid ;
				const gid_t	gid = pw.pw_gid ;
	                        u_fchown(fd,uid,gid) ; /* can fail */
			    } else if (isNotPresent(rs)) {
			        rs = SR_OK ;
			    }
			    uc_free(pwbuf) ;
			} /* end if (m-a) */
	            } /* end if (root-name) */
	        } /* end if (not SUID) */
		} /* end if (uc_fminmod) */
	        u_close(fd) ;
	    } /* end if (create-file) */
	} /* end if (MS-file did not exist) */

	if (rs >= 0) {
	    if (pip->debuglevel > 0) {
	        shio_printf(pip->efp,
	            "%s: ms=%s\n",pip->progname,lip->msfname) ;
	    }
	    if (pip->open.logprog) {
	        logfile_printf(&pip->lh,"ms=%s", lip->msfname) ;
	    }
	}

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (locinfo_msfile) */


int locinfo_reqfname(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		pl = 0 ;

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
	} else
	    pl = strlen(lip->reqfname) ;

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (locinfo_reqfname) */


int locinfo_ipcpid(LOCINFO *lip,int f)
{
	PROGINFO	*pip = lip->pip ;
	const int	of = (O_CREAT | O_WRONLY | O_TRUNC) ;
	int		rs = SR_OK ;
	cchar		*pidcname = PIDCNAME ;
	cchar		*pf ;
	char		pidfname[MAXPATHLEN + 1] ;

	if (pip->pidfname == NULL) {
	    int		pl ;

	    if ((rs = locinfo_tmpourdname(lip)) >= 0) {
	        if ((rs = mkpath2(pidfname,lip->tmpourdname,pidcname)) >= 0) {
		    cchar	**vpp = &pip->pidfname ;
	            pl = rs ;
	            rs = proginfo_setentry(pip,vpp,pidfname,pl) ;
		}
	    }

	    if ((rs < 0) && (pip->efp != NULL)) {
	        shio_printf(pip->efp,"%s: TMPDIR access problem (%d)\n",
			pip->progname,rs) ;
	    }

	} /* end if (creating PID file) */

	if (rs >= 0) {
	pf = pip->pidfname ;
	if (f) { /* activate */

	    pip->f.pidfname = FALSE ;
	    if ((rs = u_open(pf,of,0664)) >= 0) {
	        int 	fd = rs ;
	        int	wl ;
	        char	*pidbuf = pidfname ;
	        rs = ctdeci(pidbuf,MAXPATHLEN,pip->pid) ;
		wl = rs ;
	        if (rs >= 0) {
	            pidbuf[wl++] = '\n' ;
	            rs = u_write(fd,pidbuf,wl) ;
	            pip->f.pidfname = (rs >= 0) ;
	        }
	        u_close(fd) ;
	    } /* end if (file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("msulocinfo/_ipcpid: activate rs=%d\n",rs) ;
#endif

	} else { /* de-activate */

	    if ((pf != NULL) && pip->f.pidfname) {
	        pip->f.pidfname = FALSE ;
	        if (pf[0] != '\0') u_unlink(pf) ;
	    }

	} /* end if (invocation mode) */
	} /* end if (ok) */

	if ((rs < 0) && (pip->efp != NULL)) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: PID access problem (%d)\n",pn,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("msulocinfo/_ipcpid: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_ipcpid) */


int locinfo_gidrootname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->gid_rootname == 0) {
	    if ((rs = proginfo_rootname(pip)) >= 0) {
	        struct passwd	pw ;
	        const int	pwlen = getbufsize(getbufsize_pw) ;
	        char		*pwbuf ;
	        lip->gid_rootname = 0 ; /* super (unwanted) default */
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
	    debugprintf("msulocinfo_gidrootname: ret rs=%d gid=%d\n",
		rs,lip->gid_rootname) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_gidrootname) */


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


/* private subroutines */


static int locinfo_lockbeginone(LOCINFO *lip,LFM *lfp,cchar *lockfname)
{
	PROGINFO	*pip = lip->pip ;
	const mode_t	dmode = 0777 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_opened = FALSE ;
	cchar		*ccp = lockfname ;

	if ((rs >= 0) && (ccp != NULL) && (ccp[0] != '\0') && (ccp[0] != '-')) {
	    int		cl ;
	    cchar	*cp ;
	    char	tmpfname[MAXPATHLEN+1] ;

	    cl = sfdirname(lockfname,-1,&cp) ;
	    if ((rs = mkpath1w(tmpfname,cp,cl)) >= 0) {
		struct ustat	usb ;
		rs1 = u_stat(tmpfname,&usb) ;
		if ((rs1 >= 0) && (! S_ISDIR(usb.st_mode))) rs1 = SR_NOTDIR ;
		if (rs1 == SR_NOENT) {
		    rs = mkdirs(tmpfname,dmode) ;
		} else
		    rs = rs1 ;
	    }

	    if (rs >= 0) {
		LFM_CHECK	lc ;
		const int	ltype = LFM_TRECORD ;
		const int	to_lock = lip->to_lock ;
		cchar		*nn = pip->nodename ;
		cchar		*un = pip->username ;
		cchar		*bn = pip->banner ;
		rs = lfm_start(lfp,ccp,ltype,to_lock,&lc,nn,un,bn) ;
		f_opened = (rs >= 0) ;

#if	CF_DEBUG
		if (DEBUGLEVEL(4)) {
		    debugprintf("locinfo_lockbeginone: lfm_start() rs=%d\n",
			rs) ;
			sleep(10) ;
		}
#endif

#ifdef	COMMENT
	                    if ((pip->debuglevel > 0) && (rs < 0))
	                        shio_printf(pip->efp,
	                            "%s: inaccessible PID lock (%d)\n",
	                            pip->progname,rs) ;
#endif /* COMMENT */

	        if ((rs == SR_LOCKLOST) || (rs == SR_AGAIN)) {
		    msumainlockprint(pip,ccp,&lc) ;
		}
	    } /* end if */

	} /* end if (establish lock) */

	return (rs >= 0) ? f_opened : rs ;
}
/* end subroutine (locinfo_lockbeginone) */


