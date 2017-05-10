/* opensvc_logload */

/* GENSERV-user open-service (logload) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_PRNUSER	1		/* use 'prn' as user */
#define	CF_CTDECF	1		/* use 'ctdecf(3dam)' */


/* revision history:

	= 2003-11-04, David A­D­ Morano

	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.


*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an user open-service module.  This little diddy supplies a fast
	little "issue" message for programs that perform logins onto the host.

	Synopsis:

	int opensvc_logload(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<ctdecf.h>
#include	<localmisc.h>

#include	"opensvc_logload.h"
#include	"defs.h"


/* local defines */

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#ifndef	VARCLUSTER
#define	VARCLUSTER	"CLUSTER"
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	STATBUFLEN	(COLUMNS+20)
#define	MAXSTATLEN	8

#define	NETLOADLEN	STATBUFLEN
#define	SYSTATLEN	STATBUFLEN

#define	NDEBFNAME	"opensvc_logload.deb"

#define	DEFLOGUSER	"genserv"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	getnodename(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getclustername(const char *,char *,int,const char *) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	localgetnetload(const char *,char *,int) ;
extern int	localgetsystat(const char *,char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	nusers(const char *) ;
extern int	isNotPresent(int) ;

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	mkstr(char *,int,cchar *,double *,int,cchar *,cchar *) ;

static int	getloadlevel(double) ;


/* local variables */

static const char	*loadlevels[] = {
	"light",
	"moderate",
	"heavy",
	NULL
} ;


/* exported subroutines */


int opensvc_logload(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	double		dla[3] ;
	const int	ulen = USERNAMELEN ;
	const int	clen = CLUSTERNAMELEN ;
	const int	nlen = NETLOADLEN ;
	const int	slen = SYSTATLEN ;
	const int	llen = LINEBUFLEN ;
	const int	mstatlen = MIN(SYSTATLEN,MAXSTATLEN) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc = 0 ;
	int		pipes[2] ;
	int		fd = -1 ;
	int		ll = -1 ;
	int		cols = COLUMNS ;
	int		nu = 0 ;
	const char	*un = NULL ;
	const char	*varcols = getourenv(envv,VARCOLUMNS) ;
	const char	*cluster = getourenv(envv,VARCLUSTER) ;
	char		nn[NODENAMELEN+1] ;
	char		dn[MAXHOSTNAMELEN+1] ;
	char		prlocal[MAXPATHLEN+1] ;
	char		ubuf[USERNAMELEN+1] ;
	char		cbuf[CLUSTERNAMELEN+1] ;
	char		nbuf[NETLOADLEN+1] ;
	char		sbuf[SYSTATLEN+1] ;
	char		lbuf[LINEBUFLEN+2] ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_logload: entered prn=%s\n",prn) ;
#endif

/* find the number of allowable columns (for line-length limiting) */

	if (varcols != NULL) {
	    int	v ;
	    rs1 = cfdeci(varcols,-1,&v) ;
	    if (rs1 >= 0) cols = v ;
	}

/* find the username to act upon */

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	    if ((argc >= 2) && (argv[1] != '\0')) {
		un = argv[1] ;
	    }
	}

/* default user as necessary */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_logload: 0 un=%s\n",un) ;
#endif

#if	CF_PRNUSER /* more reliable in general */

	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) un = prn ;

#else /* CF_PRNUSER */

/* this is a real cheapy way to try to get the sponsoring user-name */

	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) {
	    int		pl, bl ;
	    const char	*bp ;
	    pl = strlen(pr) ;
	    while (pl && (pr[pl-1] == '/')) pl -= 1 ;
	    if ((bl = sfbasename(pr,-1,&bp)) > 0) {
		rs1 = sncpy1w(ubuf,ulen,bp,bl) ;
		if (rs1 >= 0) un = ubuf ;
	    }
	}

#endif /* CF_PRNUSER */

	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) un = DEFLOGUSER ;

/* get the program-root for the LOCAL facility */

	if (rs >= 0) {
	    if ((rs = getnodedomain(nn,dn)) >= 0) {
		rs = mkpr(prlocal,MAXPATHLEN,VARPRLOCAL,dn) ;
	    }
	}

/* get the cluster-name (as necessary) */

	if ((rs >= 0) && ((cluster == NULL) || (cluster[0] == '\0'))) {
	    if (rs >= 0) {
	        cluster = cbuf ;
	        rs = getclustername(prlocal,cbuf,clen,nn) ;
	    }
	} /* end if (cluster) */

/* get stuff */

	if (rs >= 0) {
	    if ((rs = uc_getloadavg(dla,3)) >= 0) {
	        rs = nusers(NULL) ;
	        nu = rs ;
	    }
	}

/* get the network-load */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_logload: netload rs=%d\n",rs) ;
#endif

	nbuf[0] = '\0' ;
	if (rs >= 0) {
	    rs = localgetnetload(prlocal,nbuf,nlen) ;
	    nbuf[mstatlen] = '\0' ;
	    if ((rs < 0) && isNotPresent(rs)) {
		nbuf[0] = '\0' ;
		rs = SR_OK ;
	    }
	}

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_logload: netload() rs=%d\n",rs) ;
#endif

/* get the system-status */

	sbuf[0] = '\0' ;
	if (rs >= 0) {
	    rs = localgetsystat(prlocal,sbuf,slen) ;
	    sbuf[mstatlen] = '\0' ;
	    if ((rs < 0) && isNotPresent(rs)) {
		sbuf[0] = '\0' ;
		rs = SR_OK ;
	    }
	}

/* put it all together */

	if (rs >= 0) {
	    rs = mkstr(lbuf,llen,cluster,dla,nu,nbuf,sbuf) ;
	    ll = rs ;
	}
	if (rs >= 0) {
	    if (ll > cols) ll = cols ;
	    if (ll > 0) {
	        lbuf[ll++] = '\n' ;
	        lbuf[ll] = '\0' ;
	    }
	}

/* write it out */

	if (rs >= 0) {
	if ((rs = u_pipe(pipes)) >= 0) {
	    int	wfd = pipes[1] ;
	    fd = pipes[0] ;

	    rs = u_write(wfd,lbuf,ll) ;

	    u_close(wfd) ;
	    if (rs < 0) u_close(fd) ;
	} /* end if (pipe) */
	} /* end if (ok) */

ret0:
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_logload) */


/* local subroutines */


static int mkstr(lbuf,llen,cn,dla,nu,nbuf,sbuf)
char		lbuf[] ;
int		llen ;
const char	cn[] ;
double		dla[3] ;
int		nu ;
const char	nbuf[] ;
const char	sbuf[] ;
{
	double		ddla = dla[2] ;
	int		rs ;
	int		li ;
	int		len = 0 ;
	char		dlabuf[DIGBUFLEN+1] ;

	li = getloadlevel(ddla) ;

	if (ddla > 99.9) ddla = 99.9 ;

#if	CF_CTDECF
	rs = ctdecf(dlabuf,DIGBUFLEN,ddla,'f',-1,1,-1) ;
#else
	rs = bufprintf(dlabuf,DIGBUFLEN,"%4.1f",ddla) ;
#endif /* CF_CTDECF */

	if (rs >= 0) {
	    cchar	*dp = dlabuf ;
	    cchar	*fmt ;
	    fmt = "cluster=%s loadavg(15m)=%s (%s) users=%u" ;
		" netload=%s systat=%s" ;
	    while (*dp && (dp[0] == ' ')) dp += 1 ;
	    rs = bufprintf(lbuf,llen,fmt,cn,dp,loadlevels[li],nu,nbuf,sbuf) ;
	    len = rs ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkstr) */


static int getloadlevel(double dla)
{
	int		li = 0 ;
	if (dla >= 0.33) {
	   li = 1 ;
	   if (dla >= 0.67)
		li = 2 ;
	}
	return li ;
}
/* end subroutine (getloadlevel) */


