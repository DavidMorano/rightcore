/* ow */

/* support for Open-Weather (OW) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DIRGROUP	1		/* set the GID on directories */


/* revision history:

	= 1998-07-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	These subroutines provide support for the 'openweather(3dam)'
	subroutine (facility?).


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<sys/syslog.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdarg.h>
#include	<time.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<vecstr.h>
#include	<logfile.h>
#include	<logsys.h>
#include	<format.h>
#include	<localmisc.h>

#include	"ow.h"


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	mklogid(char *,int,const char *,int,int) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	getuid_name(cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getlogfac(const char *,int) ;
extern int	getlogpri(const char *,int) ;
extern int	isNotPresent(int) ;


/* external variables */


/* local structures */


/* forward reference */

static int	ow_setextras(OW *) ;
static int	ow_mklogid(OW *) ;
static int	ow_lfopen(OW *) ;
static int	ow_lfclose(OW *) ;
static int	ow_lsopen(OW *) ;
static int	ow_lsclose(OW *) ;


/* local variables */


/* exported subroutines */


int ow_start(sip,pr,sn,vd,ws,daytime,of,to)
OW		*sip ;
const char	*pr ;
const char	*sn ;
const char	*vd ;
const char	*ws ;
time_t		daytime ;
int		of ;
int		to ;
{
	int	rs ;

	if (sip == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("ow_start: entered ws=%s\n",ws) ;
#endif

	memset(sip,0,sizeof(OW)) ;
	sip->pr = pr ;
	sip->sn = sn ;
	sip->vd = vd ;
	sip->ws = ws ;
	sip->ld = OW_LDNAME ;
	sip->daytime = daytime ;
	sip->oflags = of ;
	sip->to = to ;
	sip->intpoll = OW_INTSHORT ;
	sip->pruid = -1 ;
	sip->prgid = -1 ;
	sip->wmin = -1 ;
	sip->wfd = -1 ;
	sip->gid_rootname = -1 ;

/* flags to start with */

	sip->f.lf = TRUE ;		/* LOGFILE enabled */

/* the rest of it */

	if ((rs = vecstr_start(&sip->stores,4,0)) >= 0) {
	    sip->f.stores = TRUE ;
	    if ((rs = vecstr_start(&sip->svars,4,0)) >= 0) {
	        sip->f.svars = TRUE ;
	        rs = ow_setextras(sip) ;
	        if (rs < 0) {
	            sip->f.svars = FALSE ;
		    vecstr_finish(&sip->svars) ;
		}
	    }
	    if (rs < 0) {
	        sip->f.stores = FALSE ;
	        vecstr_finish(&sip->stores) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("ow_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ow_start) */


int ow_finish(sip,f_abort)
OW		*sip ;
int		f_abort ;
{
	int	rs = SR_OK ;
	int	rs1 ;


#if	CF_DEBUGS
	debugprintf("ow_finish: f_abort=%u\n",f_abort) ;
#endif

	if (sip->open.ls) {
	    rs1 = ow_lsclose(sip) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->open.lf) {
	    rs1 = ow_lfclose(sip) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (f_abort && (sip->wfd >= 0)) {
	    u_close(sip->wfd) ;
	    sip->wfd = -1 ;
	}

	if (sip->f.svars) {
	    sip->f.svars = FALSE ;
	    rs1 = vecstr_finish(&sip->svars) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->f.stores) {
	    sip->f.stores = FALSE ;
	    rs1 = vecstr_finish(&sip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	memset(sip,0,sizeof(OW)) ;

#if	CF_DEBUGS
	debugprintf("ow_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ow_finish) */


int ow_setentry(sip,epp,vp,vl)
OW		*sip ;
const char	**epp ;
const char	*vp ;
int		vl ;
{
	int	rs = SR_OK ;
	int	oi, i ;
	int	vnlen = 0 ;

	const char	*cp ;


	if (sip == NULL)
	    return SR_FAULT ;

	if (epp == NULL)
	    return SR_INVALID ;

/* find existing entry for later deletion */

	oi = -1 ;
	if (*epp != NULL)
	    oi = vecstr_findaddr(&sip->stores,*epp) ;

/* add the new entry */

	if (vp != NULL) {
	    vnlen = strnlen(vp,vl) ;

	    rs = vecstr_add(&sip->stores,vp,vnlen) ;
	    i = rs ;
	    if (rs >= 0) {
	        rs = vecstr_get(&sip->stores,i,&cp) ;
	        if (rs >= 0) *epp = cp ;
	    } /* end if (added new entry) */

	} /* end if (had a new entry) */

/* delete the old entry if we had one */

	if ((rs >= 0) && (oi >= 0))
	    vecstr_del(&sip->stores,oi) ;

	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (ow_setentry) */


int ow_nodedomain(sip)
OW		*sip ;
{
	int	rs = SR_OK ;


	if (sip->nodename == NULL) {
	    char	nn[MAXHOSTNAMELEN+1] ;
	    char	dn[MAXHOSTNAMELEN+1] ;
	    rs = getnodedomain(nn,dn) ;
	    if (rs >= 0)
	        rs = ow_setentry(sip,&sip->nodename,nn,-1) ;
	    if (rs >= 0)
	        rs = ow_setentry(sip,&sip->domainname,dn,-1) ;
	}

	return rs ;
}
/* end subroutine (ow_nodedomain) */


int ow_setmin(sip)
OW		*sip ;
{
	int	rs = SR_OK ;


	if (sip->wmin < 0) {
	    struct tm	ts ;
	    if (sip->daytime == 0) sip->daytime = time(NULL) ;
	    rs = uc_gmtime(&sip->daytime,&ts) ;
	    if (rs >= 0) sip->wmin = ts.tm_min ;
	}

	return rs ;
}
/* end subroutine (ow_setmin) */


int ow_isvalid(sip,ft)
OW		*sip ;
time_t		ft ;
{
	int	rs = SR_OK ;
	int	intstale = OW_INTLONG ;
	int	f_valid = FALSE ;


	if (sip->wmin < 0) rs = ow_setmin(sip) ;

	if (rs >= 0) {
	    int	wmin = sip->wmin ;
	    int	f_inwindow ;

	    f_inwindow = ((wmin > 50) && (wmin <= 10)) ;
	    if (f_inwindow) intstale = OW_INTSHORT ;

	    f_valid = ((ft > 0) && ((sip->daytime - ft) < intstale)) ;

	} /* end if */

	return (rs >= 0) ? f_valid : rs ;
}
/* end subroutine (ow_isvalid) */


int ow_prid(sip)
OW		*sip ;
{
	int	rs = SR_OK ;


	if (sip->pruid < 0) {
	    struct ustat	sb ;
	    rs = u_stat(sip->pr,&sb) ;
	    if (rs >= 0) {
	        sip->pruid = sb.st_uid ;
	        sip->prgid = sb.st_gid ;
	    }
	}

	return rs ;
}
/* end subroutine (ow_prid) */


int ow_logprintf(OW *sip,const char *fmt,...)
{
	va_list	ap ;

	const int	m = 0 ; /* mode==0 */
	const int	dlen = LINEBUFLEN ;

	int	rs = SR_OK ;
	int	dl = 0 ;

	char	dbuf[LINEBUFLEN+1] ;


	if (sip == NULL)
	    return SR_FAULT ;

	if (fmt == NULL)
	    return SR_FAULT ;

	if ((rs >= 0) && sip->f.lf && (! sip->open.lf))
	    rs = ow_lfopen(sip) ;

	if ((rs >= 0) && sip->f.ls && (! sip->open.ls))
	    rs = ow_lsopen(sip) ;

	if (rs >= 0) {
	    va_begin(ap,fmt) ;
	    rs = format(dbuf,dlen,m,fmt,ap) ;
	    dl = rs ;
	    va_end(ap) ;
	}

	if ((rs >= 0) && sip->f.lf && sip->open.lf && (dl > 0)) {
	    logfile_write(&sip->lf,dbuf,dl) ;
	}

	if ((rs >= 0) && sip->f.ls && sip->open.ls && (dl > 0)) {
	    const int	logpri = LOG_NOTICE ;
	    logsys_write(&sip->ls,logpri,dbuf,dl) ;
	}

	return (rs >= 0) ? dl : rs ;
}
/* end subroutine (ow_logprintf) */


#ifdef	COMMENT

int ow_tmpourdname(sip)
OW		*sip ;
{
	struct ustat	usb ;

	mode_t	dmode = 0775 ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	pl = 0 ;
	int	f_needmode = FALSE ;
	int	f_created = FALSE ;
	int	f_runasprn = FALSE ;

	const char	*tn = sip->tmpdname ;
	const char	*rn = NULL ;
	const char	*sn = sip->sn ;

	char	tmpourdname[MAXPATHLEN + 1] ;


	tmpourdname[0] = '\0' ;		/* just a gesture of safety! */
	if (sip->tmpourdname != NULL) {
	    pl = strlen(sip->tmpourdname) ;
	    goto ret0 ;
	}

/* operation in the following block must remain in this order */

	{
	    if ((rs >= 0) && (sip->rootname == NULL))
	        rs = proginfo_rootname(sip) ;
	    rn = sip->rootname ;
	}

#if	CF_DEBUGS
	    debugprintf("ow_tmpourdname: rn=%s\n",rn) ;
#endif

/* set the tmp-dir permission mode depending on how we are running */

	f_runasprn = (strcmp(sip->username,sip->rootname) == 0) ;
	if (! f_runasprn) dmode = 0777 ;

#if	CF_DEBUGS
	    debugprintf("ow_tmpourdname: f_runasprn=%u\n",
	        f_runasprn) ;
#endif

/* create the tmp-dir as needed */

	rs1 = SR_OK ;
	if (rs >= 0) {
	    rs = mkpath3(tmpourdname,tn,rn,sn) ;
	    pl = rs ;
	    if (rs >= 0) {
	        rs1 = u_stat(tmpourdname,&usb) ;
	        if (rs1 >= 0) {
	            if (S_ISDIR(usb.st_mode)) {
	                const int	am = (R_OK|W_OK|X_OK) ;
	                f_needmode = ((usb.st_mode & dmode) != dmode) ;
	                rs = u_access(tmpourdname,am) ;
	            } else
	                rs = SR_NOTDIR ;
	        }
	    }
	}

	if (rs < 0)
	    goto ret0 ;

	if ((rs >= 0) && (rs1 == SR_NOENT)) {
	    f_needmode = TRUE ;

	    if (rs >= 0) {
	        f_created = TRUE ;
	        rs = mkdirs(tmpourdname,dmode) ;
	    }

	} /* end if (there was no-entry) */

	if ((rs >= 0) && f_needmode)
	    rs = uc_minmod(tmpourdname,dmode) ;

#if	CF_DIRGROUP
	if ((rs >= 0) && f_created) {
	    if ((rs = ow_gidrootname(sip)) >= 0) {
	        if (! f_runasprn) {
	            uid_t	uid_prn = 0 ;
		    if ((rs = getuid_name(rn,-1)) >= 0) {
	                uid_prn = rs ;
		    } else if (isNotPresent(rs)) {
	                uid_prn = sip->euid ;
			rs = SR_OK ;
		    }
		    if (rs >= 0) {
	                rs = u_chown(tmpourdname,uid_prn,sip->gid_rootname) ;
		    }
	        } else {
	            rs = u_chown(tmpourdname,-1,sip->gid_rootname) ;
		}
	    }
	}
#endif /* CF_DIRGROUP */

ret0:
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (ow_tmpourdname) */

#endif /* COMMENT */


#ifdef	COMMENT

int ow_checklock(sip,lfp)
OW		*sip ;
LFM		*lfp ;
{
	LFM_CHECK	ci ;

	int	rs = SR_OK ;


	if ((lfp == NULL) || (! sip->open.pidlock))
	    goto ret0 ;

	rs = lfm_check(lfp,&ci,sip->daytime) ;

#if	CF_DEBUGS
	    debugprintf("ow/_checklock: lfm_check() rs=%d\n",
	        rs) ;
	    if (rs < 0) {
	        debugprintf("ow/_checklock: pid=%d\n",ci.pid) ;
	        debugprintf("ow/_checklock: node=%s\n",
	            ci.nodename) ;
	        debugprintf("ow/_checklock: user=%s\n",
	            ci.username) ;
	        debugprintf("ow/_checklock: banner=%s\n",
	            ci.banner) ;
	    }
#endif /* CF_DEBUGS */

	if (rs == SR_LOCKLOST) {
	    msumainlockprint(sip,&ci) ;
	}

ret0:
	return rs ;
}
/* end subroutine (ow_checklock) */


int ow_reqfname(sip)
OW		*sip ;
{
	int	rs = SR_OK ;
	int	pl = 0 ;

	const char	*reqcname = REQCNAME ;

	char	reqfname[MAXPATHLEN + 1] ;


	if (sip->reqfname != NULL) {
	    pl = strlen(sip->reqfname) ;
	    goto ret0 ;
	}

	rs = ow_tmpourdname(sip) ;
	if (rs < 0)
	    goto ret0 ;

	rs = mkpath2(reqfname,sip->tmpourdname,reqcname) ;
	pl = rs ;
	if (rs >= 0) {
	    rs = ow_setentry(sip,&sip->reqfname,reqfname,pl) ;
	}

ret0:
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (ow_reqfname) */


int ow_ipcpid(sip,f)
OW		*sip ;
int		f ;
{
	int	rs = SR_OK ;
	int	oflags = (O_CREAT | O_WRONLY | O_TRUNC) ;

	const char	*pidcname = PIDCNAME ;
	const char	*pf ;

	char	pidfname[MAXPATHLEN + 1] ;


	if (sip->pidfname == NULL) {
	    int		pl ;

	    rs = ow_tmpourdname(sip) ;

#if	CF_DEBUGS
	        debugprintf("ow/_ipcpid: _tmpourdname() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = mkpath2(pidfname,sip->tmpourdname,pidcname) ;
	        pl = rs ;
	    }

	    if (rs >= 0)
	        rs = ow_setentry(sip,&sip->pidfname,pidfname,pl) ;

	    if ((rs < 0) && (sip->efp != NULL))
	        shio_printf(sip->efp,"%s: TMPDIR access problem (%d)\n",
	            sip->progname,rs) ;

	} /* end if (creating PID file) */

	if (rs < 0)
	    goto ret1 ;

	pf = sip->pidfname ;

	if (f) { /* activate */

	    sip->f.pidfname = FALSE ;
	    if ((rs = u_open(pf,oflags,0664)) >= 0) {
	        int 	fd = rs ;
	        int	wl ;
	        char	*pidbuf = pidfname ;
	        rs = ctdeci(pidbuf,MAXPATHLEN,sip->pid) ;
	        wl = rs ;
	        if (rs >= 0) {
	            pidbuf[wl++] = '\n' ;
	            rs = u_write(fd,pidbuf,wl) ;
	            sip->f.pidfname = (rs >= 0) ;
	        }
	        u_close(fd) ;
	    }

#if	CF_DEBUGS
	        debugprintf("ow/_ipcpid: activate rs=%d\n",rs) ;
#endif

	} else { /* de-activate */

	    if ((pf != NULL) && sip->f.pidfname) {
	        sip->f.pidfname = FALSE ;
	        if (pf[0] != '\0') u_unlink(pf) ;
	    }

	} /* end if (invocation mode) */

ret1:
	if ((rs < 0) && (sip->efp != NULL))
	    shio_printf(sip->efp,"%s: PID access problem (%d)\n",
	        sip->progname,rs) ;

ret0:

#if	CF_DEBUGS
	    debugprintf("ow/_ipcpid: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ow_ipcpid) */


int ow_gidrootname(sip)
OW		*sip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->gid_rootname < 0) {
	if ((rs = proginfo_rootname(sip)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("ow_gidrootname: rn=%s\n",sip->rootname) ;
#endif

	sip->gid_rootname = 0 ; /* super (unwanted) default */

	    if ((rs = getuid_name(sip->rootname,-1)) >= 0) {
		sip->gid_rootname = rs ;
	    } else if (isNotPresent(rs)) {
	        char	dname[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(dname,sip->tmpdname,sip->rootname)) >= 0) {
	            struct ustat	sb ;
	            if ((rs1 = u_stat(dname,&sb)) >= 0) {
	                if (S_ISDIR(sb.st_mode)) {
	                    sip->gid_rootname = sb.st_gid ;
	                } else 
	                    rs = SR_NOTDIR ;
	            }
	        } /* end if (mkpath) */
	    }
	} /* end if (needed) */

#if	CF_DEBUGS
	    debugprintf("ow_gidrootname: ret rs=%d gid=%d\n",
	        rs,sip->gid_rootname) ;
#endif

	return rs ;
}
/* end subroutine (ow_gidrootname) */


/* calculate a file name */
int ow_setfname(sip,fname,ebuf,el,f_def,dname,name,suf)
OW		*sip ;
char		fname[] ;
const char	ebuf[] ;
const char	dname[], name[], suf[] ;
int		el ;
int		f_def ;
{
	int	rs = 0 ;
	int	ml ;
	int	fl = 0 ;

	const char	*np ;

	char	tmpname[MAXNAMELEN + 1] ;


	if ((f_def && (ebuf[0] == '\0')) || (strcmp(ebuf,"+") == 0)) {
	    np = name ;
	    if ((suf != NULL) && (suf[0] != '\0')) {
	        np = tmpname ;
	        mkfnamesuf1(tmpname,name,suf) ;
	    }
	    if (np[0] != '/') {
	        if ((dname != NULL) && (dname[0] != '\0')) {
	            rs = mkpath3(fname,sip->pr,dname,np) ;
	        } else
	            rs = mkpath2(fname, sip->pr,np) ;
	    } else
	        rs = mkpath1(fname, np) ;
	    fl = rs ;
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
	        }
	    } else
	        rs = mkpath1(fname,np) ;
	    fl = rs ;
	} /* end if */

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (ow_setfname) */

#endif /* COMMENT */


/* local subroutines */


static int ow_setextras(sip)
OW		*sip ;
{
	int	rs = SR_OK ;


	if (sip->rn == NULL) {
	    sip->pid = getpid() ;
	    {
	        const char	*rnp ;
	        int		rnl ;
	        rnl = sfbasename(sip->pr,-1,&rnp) ;
	        if (rnl == 0) {
	            rnp = "root" ;
	            rnl = 4 ;
	        }
	        rs = ow_setentry(sip,&sip->rn,rnp,rnl) ;
	    }
	} /* end if (initialization needed) */

	return rs ;
}
/* end subroutine (ow_setextras) */


static int ow_lfopen(OW *sip)
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	f_open = FALSE ;

	const char	*ln = sip->logfname ;


	if (sip->f.lf) {
	    int	f = FALSE ;
	    f = f || (ln == NULL) ;
	    f = f || (ln[0] == '\0') ;
	    f = f || (ln[0] == '+') ;
	    if (f) {
	        ln = sip->sn ;
	    } else if (ln[0] == '-') {
	        ln = NULL ;
	        sip->f.lf = FALSE ;
	    }
	}

	if (sip->f.lf) {
	    char	lfname[MAXPATHLEN+1] ;
	    if ((rs >= 0) && (ln != NULL))
	        rs = mkpath3(lfname,sip->pr,sip->ld,ln) ;
	    if (rs >= 0)
	        rs = ow_mklogid(sip) ;
	    if (rs >= 0) {
	        rs1 = logfile_open(&sip->lf,lfname,0,0666,sip->logid) ;
	        f_open = (rs1 >= 0) ;
	        sip->open.lf = f_open ;
	        if (! isNotPresent(rs1)) rs = rs1 ;
	    }
	}

	return (rs >= 0) ? f_open : rs ;
}
/* end subroutine (ow_lfopen) */


static int ow_lfclose(OW *sip)
{
	int	rs = SR_OK ;


	if (sip->open.lf) {
	    sip->open.lf = FALSE ;
	    rs = logfile_close(&sip->lf) ;
	}

	return rs ;
}
/* end subroutine (ow_lfclose) */


static int ow_lsopen(OW *sip)
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	f_open = FALSE ;


	if (sip == NULL) return SR_FAULT ;

	if (sip->f.ls) {
	    int	logfac = LOG_DAEMON ;
	    rs = ow_mklogid(sip) ;
	    if (rs >= 0) {
	        const char	*logtag = sip->sn ;
	        const char	*logid = sip->logid ;
	        rs1 = logsys_open(&sip->ls,logfac,logtag,logid,0) ;
	        f_open = (rs1 >= 0) ;
	        sip->open.ls = f_open ;
	    }
	}

	return (rs >= 0) ? f_open : rs ;
}
/* end subroutine (ow_lsopen) */


static int ow_lsclose(OW *sip)
{
	int	rs = SR_OK ;


	if (sip->open.ls) {
	    sip->open.ls = FALSE ;
	    rs = logsys_close(&sip->ls) ;
	}

	return rs ;
}
/* end subroutine (ow_lsclose) */


static int ow_mklogid(OW *sip)
{
	int	rs = SR_OK ;


	if (sip->logid[0] == '\0') {
	    const int	logidlen = LOGFILE_LOGIDLEN ;

	    if (sip->nodename == NULL)
	        rs = ow_nodedomain(sip) ;

	    if (rs >= 0) {
	        const char	*nn = sip->nodename ;
	        int		v = sip->pid ;
	        rs = mklogid(sip->logid,logidlen,nn,-1,v) ;
	    }

	} /* end if (needed to make it) */

	return rs ;
}
/* end subroutine (ow_mklogid) */


