/* owmain */

/* open a file-descriptor to a weather METAR */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DIRGROUP	1		/* set GID on directories */


/* revision history:

	= 1998-07-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine opens a file that contains a weather METAR.
	Only the most current METAR is returned.

	Synopsis:

	int openweather(const char *pr,const char *ws,int oflags,int to)

	Arguments:

	pr		program-root
	ws		weather station identification (eg: "kbos")
	oflags		open flags (see 'open(2)')
	to		time-out

	Returns:

	>=0		file descriptor to program STDIN and STDOUT
	<0		error

	Note that all METARs are read-only.  Regardless of the type
	of open-flags supplied only a read-only file-descriptor (FD)
	is returned.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<linefold.h>
#include	<logfile.h>
#include	<logsys.h>
#include	<localmisc.h>

#include	"ow.h"
#include	"owconfig.h"


/* local defines */

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	WSBUFLEN
#define	WSBUFLEN	16
#endif


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	mkpath5(char *,const char *,const char *,const char *,
			const char *,const char *) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	dialhttp(const char *,const char *,int,
			const char *,const char **, int,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward reference */

static int	ow_avail(OW *) ;
static int	ow_loadsvars(OW *) ;
static int	ow_findconf(OW *) ;
static int	ow_setdefaults(OW *) ;
static int	ow_openweb(OW *) ;
static int	ow_procweb(OW *,int) ;
static int	ow_checkdirs(OW *) ;
static int	ow_checkdir(OW *,const char *,mode_t) ;


/* local variables */

static const char	*schedconf[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	NULL
} ;


/* exported subroutines */


int openweather(pr,ws,of,to)
const char	*pr ;
const char	*ws ;
int		of ;
int		to ;
{
	OW		si, *sip = &si ;
	time_t		ti_now = time(NULL) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		fd = -1 ;
	const char	*sn = OW_SEARCHNAME ;
	const char	*vd = OW_VDNAME ;

	if (pr == NULL) return SR_FAULT ;
	if (ws == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("openweather: pr=%s\n",pr) ;
	debugprintf("openweather: ws=%s\n",ws) ;
#endif

	if ((rs = ow_start(sip,pr,sn,vd,ws,ti_now,of,to)) >= 0) {

	    if ((ws != NULL) && (ws[0] != '\0')) {
	        rs = ow_avail(sip) ;
	        fd = rs ;

#if	CF_DEBUGS
	debugprintf("openweather: ow_avail() rs=%d fd=%u\n",rs,fd) ;
#endif

	    }

	    if ((rs < 0) && (isNotPresent(rs) || (rs == SR_STALE))) {
	        const char	*cf ;

	        rs = ow_loadsvars(sip) ;

#if	CF_DEBUGS
	debugprintf("openweather: ow_loadsvars() rs=%d\n",rs) ;
#endif
	        if (rs >= 0) {
	            rs = ow_findconf(sip) ; /* sets 'cfname' */
#if	CF_DEBUGS
	debugprintf("openweather: ow_findconf() rs=%d\n",rs) ;
#endif
		}

	        cf = sip->cfname ;
		if (rs >= 0) {
	        if ((rs = owconfig_start(&sip->c,sip,cf)) >= 0) {

#if	CF_DEBUGS
	debugprintf("openweather: owconfig_start() rs=%d\n",rs) ;
#endif
	            rs = ow_setdefaults(sip) ;

	            if (rs >= 0)
	                rs = ow_checkdirs(sip) ;

	            if (rs >= 0)
	                rs = ow_openweb(sip) ;

	            rs1 = owconfig_finish(&sip->c) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (owconfig) */
		} /* end if */

#if	CF_DEBUGS
	debugprintf("openweather: owconfig-out rs=%d\n",rs) ;
#endif
	    } /* end if (ow) */

	    rs1 = ow_finish(sip,(rs<0)) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	} /* end if (file not present or was stale) */

#if	CF_DEBUGS
	debugprintf("openweather: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openweather) */


/* local subroutines */


static int ow_avail(sip)
OW		*sip ;
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		fd = -1 ;
	const char	*pr = sip->pr ;
	const char	*sn = sip->sn ;
	const char	*vd = sip->vd ;
	const char	*ws = sip->ws ;
	const char	*mdname = OW_METARDNAME ;
	char		wsfname[MAXPATHLEN+1] ;

	if (ws == NULL) {
	    rs = SR_NOENT ;
	    goto ret0 ;
	}

/* we should not be called except before anything other attempts */

	if ((sip->ti_weather > 0) || (sip->wfd >= 0)) {
	    rs = SR_NOANODE ;
	    goto ret0 ;
	}

/* continue with this (first) attempt */

	rs = mkpath5(wsfname,pr,vd,sn,mdname,ws) ;
	if (rs < 0) goto ret0 ;

	rs = u_open(wsfname,O_RDONLY,0666) ;
	fd = rs ;
	if (rs >= 0) {
	    rs = u_fstat(fd,&sb) ;
	    if (rs >= 0) {
	        sip->wfd = fd ;
	        sip->ti_weather = sb.st_mtime ;
	        rs = ow_isvalid(sip,sb.st_mtime) ;
	        if (rs == 0) rs = SR_STALE ;
	    } else if (fd >= 0)
	        u_close(fd) ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("ow_avail: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (ow_avail) */


static int ow_loadsvars(sip)
OW		*sip ;
{
	VECSTR	*slp = &sip->svars ;

	int	rs = SR_OK ;


	if (sip->nodename == NULL)
	    rs = ow_nodedomain(sip) ;

	if (rs >= 0) {
	    if ((rs = vecstr_count(slp)) == 0) {

	        if (rs >= 0)
	            rs = vecstr_envadd(slp,"p",sip->pr,-1) ;

	        if (rs >= 0)
	            rs = vecstr_envadd(slp,"n",sip->sn,-1) ;

	    } /* end if */
	} /* end if */

	return rs ;
}
/* end subroutine (ow_loadsvars) */


static int ow_findconf(sip)
OW		*sip ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	pl = 0 ;

	const char	*cf = OW_CFNAME ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if (sip->cfname == NULL) {
	    vecstr	*slp = &sip->svars ;

	    rs1 = permsched(schedconf,slp,tmpfname,MAXPATHLEN,cf,R_OK) ;
	    if (rs1 >= 0) {
	        pl = rs1 ;
	        rs = ow_setentry(sip,&sip->cfname,tmpfname,pl) ;
	    }

	} /* end if */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (ow_findconf) */


static int ow_setdefaults(sip)
OW		*sip ;
{
	int	rs = SR_OK ;


	if (sip->ws == NULL) {
	    if (sip->defws != NULL) sip->ws = sip->defws ;
	    if (sip->ws == NULL) sip->ws = OW_WS ;
	}

	if (sip->whost == NULL) {
	    sip->whost = OW_WHOST ;
	}

	if (sip->wprefix == NULL) {
	    sip->wprefix = OW_WPREFIX ;
	}

	if (sip->to == 0)
	    sip->to = OW_TO ;

	if (sip->logfacility == NULL)
	    sip->logfacility = OW_LOGFACILITY ;

	return rs ;
}
/* end subroutine (ow_setdefaults) */


static int ow_openweb(sip)
OW		*sip ;
{
	const int	af = AF_UNSPEC ;
	const int	opts = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		to = sip->to ;
	int		fd = -1 ;
	const char	*wh = sip->whost ;
	const char	*wps = OW_WEBPORT ;
	const char	*wprefix = sip->wprefix ;
	const char	*ws = sip->ws ;
	const char	**wsvcargs = NULL ;
	char		wsbuf[WSBUFLEN+1], *wp = wsbuf ;
	char		svc[MAXNAMELEN+1] ;

	if (ws == NULL) return SR_FAULT ;

	if (ws[0] == '\0') return SR_INVALID ;

/* make the weather-station part upper-case */

	{
	    int	ml = (WSBUFLEN-4) ;
	    wp = strwcpyuc(wsbuf,ws,ml) ;
	    strwcpy(wp,".TXT",4) ;
	}

/* put it together with the prefix */

	rs = sncpy3(svc,MAXNAMELEN,wprefix,"/",wsbuf) ;
	if (rs < 0) goto ret0 ;

/* dial out */

	if (rs >= 0) {
	    rs1 = dialhttp(wh,wps,af,svc,wsvcargs,to,opts) ;
	    if (rs1 >= 0) {
	        int	wfd = rs ;
	        rs = ow_procweb(sip,wfd) ;
	        fd = rs ;
	        u_close(wfd) ;
	    }
	}

ret0:
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (ow_openweb) */


static int ow_procweb(sip,wfd)
OW		*sip ;
int		wfd ;
{
	int	rs = SR_OK ;



	return rs ;
}
/* end subroutine (ow_procweb) */


static int ow_checkdirs(sip)
OW		*sip ;
{
	int	rs = SR_OK ;

	const char	*pr = sip->pr ;
	const char	*vd = sip->vd ;
	const char	*sn = sip->sn ;
	const char	*md = OW_METARDNAME ;

	char	wdname[MAXPATHLEN + 1] ;
	char	mdname[MAXPATHLEN + 1] ;


	if (rs >= 0) {
	    rs = mkpath3(wdname,pr,vd,sn) ;
	    if (rs >= 0)
	        rs = ow_checkdir(sip,wdname,0775) ;
	}

	if (rs >= 0) {
	    rs = mkpath2(mdname,wdname,md) ;
	    if (rs >= 0)
	        rs = ow_checkdir(sip,mdname,0777) ;
	}

ret0:
	return rs ;
}
/* end subroutine (ow_checkdirs) */


static int ow_checkdir(sip,dname,m)
OW		*sip ;
const char	dname[] ;
mode_t		m ;
{
	struct ustat	usb ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f_needmode = FALSE ;
	int	f_created = FALSE ;


	m &= S_IAMB ;
	rs1 = u_stat(dname,&usb) ;
	if (rs1 >= 0) {
	    if (S_ISDIR(usb.st_mode)) {
	        const int	am = (R_OK|W_OK|X_OK) ;
	        f_needmode = ((usb.st_mode & m) != m) ;
	        rs = u_access(dname,am) ;
	    } else
	        rs = SR_NOTDIR ;
	}

	if (rs < 0)
	    goto ret0 ;

	if ((rs >= 0) && (rs1 == SR_NOENT)) {

	    f_needmode = TRUE ;
	    f_created = TRUE ;
	    rs = mkdirs(dname,m) ;

	} /* end if (there was no-entry) */

	if ((rs >= 0) && f_needmode)
	    rs = uc_minmod(dname,m) ;

#if	CF_DIRGROUP
	if ((rs >= 0) && f_created) {
	    rs = ow_prid(sip) ;
	    if (rs >= 0) rs = u_chown(dname,-1,sip->prgid) ;
	}
#endif /* CF_DIRGROUP */

ret0:
	return (rs >= 0) ? f_created : rs ;
}
/* end subroutine (ow_checkdir) */



