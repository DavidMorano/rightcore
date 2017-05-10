/* localgetorg */

/* get the organization name (string) for a specified user-name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_USERORG	0		/* try to access "user" org also */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine retrieves the organization name (string) for a specified
        user-name.

	Synopsis:

	int localgetorg(pr,rbuf,rlen,username)
	const char	*pr ;
	char		rbuf[] ;
	int		rlen ;
	const char	*username ;

	Arguments:

	pr		program-root
	rbuf		user supplied buffer to hold result
	rlen		length of user supplied buffer
	username	username to look up

	Returns:

	>=0		length of return organization string
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<char.h>
#include	<filebuf.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<gecos.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#define	SUBINFO		struct subinfo

#ifndef	LOCALUSERNAME
#define	LOCALUSERNAME	"local"
#endif

#ifndef	ETCDNAME
#define	ETCDNAME	"/etc"
#endif

#undef	ORGCNAME
#define	ORGCNAME	"organization"

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARORGANIZATION
#define	VARORGANIZATION	"ORGANIZATION"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	readfileline(char *,int,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct subinfo {
	const char	*pr ;
	const char	*prn ;		/* program-root name */
	const char	*ofn ;		/* organization file-name */
	const char	*un ;
	char		*rbuf ;		/* user-supplied buffer */
	int		rlen ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,cchar *,cchar *,cchar *,char *,int) ;
static int	subinfo_homer(SUBINFO *,const char *) ;
static int	subinfo_passwder(SUBINFO *,const char *) ;
static int	subinfo_finish(SUBINFO *) ;

#if	CF_USERORG
static int	localgetorg_var(SUBINFO *) ;
static int	localgetorg_home(SUBINFO *) ;
static int	localgetorg_passwd(SUBINFO *) ;
#endif /* CF_USERORG */

static int	localgetorg_prhome(SUBINFO *) ;
static int	localgetorg_pretc(SUBINFO *) ;
static int	localgetorg_prpasswd(SUBINFO *) ;
static int	localgetorg_sys(SUBINFO *) ;


/* local variables */


/* exported subroutines */


int localgetorg(cchar *pr,char *rbuf,int rlen,cchar *username)
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	const char	*ofn = ORGCNAME ;

	if ((pr == NULL) || (rbuf == NULL) || (username == NULL))
	    return SR_FAULT ;

	rbuf[0] = '\0' ;
	if (username[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("localgetorg: pr=%s\n",pr) ;
	debugprintf("localgetorg: u=%s\n",username) ;
#endif

	if ((rs = subinfo_start(&si,pr,ofn,username,rbuf,rlen)) >= 0) {
	    int	i ;

	    for (i = 0 ; i < 7 ; i += 1) {
		rs = 0 ;
		switch (i) {
#if	CF_USERORG
		case 0:
		     rs = localgetorg_var(sip) ;
		     break ;
		case 1:
		     rs = localgetorg_home(sip) ;
		     break ;
		case 2:
		     rs = localgetorg_passwd(sip) ;
		     break ;
#endif /* CF_USERORG */
		case 3:
		     rs = localgetorg_prhome(sip) ;
		     break ;
		case 4:
		     rs = localgetorg_pretc(sip) ;
		     break ;
		case 5:
		     rs = localgetorg_prpasswd(sip) ;
		     break ;
		case 6:
		     rs = localgetorg_sys(sip) ;
		     break ;
		} /* end switch */
		len = rs ;

#if	CF_DEBUGS
	debugprintf("localgetorg: localgetorg_%u() rs=%d\n",i,rs) ;
#endif

		if ((rs < 0) && (! isNotPresent(rs))) break ;
		if (len > 0) break ;
	    } /* end for */

	    rs1 = subinfo_finish(&si) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("localgetorg: ret rs=%d len=%u\n",rs,len) ;
	debugprintf("localgetorg: org=>%s<\n",rbuf) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (localgetorg) */


/* local subroutines */


static int subinfo_start(sip,pr,ofn,un,rbuf,rlen)
SUBINFO		*sip ;
const char	*pr ;
const char	*ofn ;
const char	*un ;
char		*rbuf ;
int		rlen ;
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*cp ;
	const char	*ccp ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;
	sip->un = un ;
	sip->ofn = ofn ;
	sip->rbuf = rbuf ;
	sip->rlen = rlen ;
	cl = sfbasename(sip->pr,-1,&cp) ;

	if (cl > 0) {
	    while ((cl > 0) && (cp[cl-1] == '/')) cl -= 1 ;
	}
	if (cl <= 0) {
	    cp = LOCALUSERNAME ;
	    cl = -1 ;
	}
	rs = uc_mallocstrw(cp,cl,&ccp) ;
	if (rs >= 0) sip->prn = ccp ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip == NULL)
	    return SR_FAULT ;

	if (sip->prn != NULL) {
	    rs1 = uc_free(sip->prn) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->prn = NULL ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


#if	CF_USERORG

static int localgetorg_var(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		f ;
	int		len = 0 ;
	const char	*un = sip->un ;

	f = (un[0] == '-') ;
	if (! f) {
	    const char	*vun = getenv(VARUSERNAME) ;
	    if ((vun != NULL) && (vun[0] != '\0'))
	        f = (strcmp(vun,un) == 0) ;
	}
	if (f) {
	    const char	*orgp = getenv(VARORGANIZATION) ;
	    if (orgp != NULL) {
		rs = sncpy1(sip->rbuf,sip->rlen,orgp) ;
		len = rs ;
	    }
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (localgetorg_var) */


static int localgetorg_home(SUBINFO *sip)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("localgetorg_home: un=%s\n",sip->un) ;
#endif

	rs = subinfo_homer(sip,sip->un) ;

#if	CF_DEBUGS
	debugprintf("localgetorg_home: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (localgetorg_home) */


static int localgetorg_passwd(SUBINFO *sip)
{

	return subinfo_passwder(sip,sip->un) ;
}
/* end subroutine (localgetorg_passwd) */

#endif /* CF_USERORG */


static int localgetorg_prhome(SUBINFO *sip)
{

	return subinfo_homer(sip,sip->prn) ;
}
/* end subroutine (localgetorg_prhome) */


static int localgetorg_prpasswd(SUBINFO *sip)
{

	return subinfo_passwder(sip,sip->prn) ;
}
/* end subroutine (localgetorg_prpasswd) */


static int localgetorg_pretc(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	char		orgfname[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("localgetorg_pretc: pr=%s\n",sip->pr) ;
#endif

	if ((rs = mkpath3(orgfname,sip->pr,ETCDNAME,sip->ofn)) >= 0) {
	    if ((rs = readfileline(sip->rbuf,sip->rlen,orgfname)) >= 0) {
	        len = rs ;
	    } else if (isNotPresent(rs))
		rs = SR_OK ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (localgetorg_pretc) */


static int localgetorg_sys(SUBINFO *sip)
{
	int		rs ;
	int		len = 0 ;
	char		orgfname[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("localgetorg_sys: pr=%s\n",sip->pr) ;
#endif

	if ((rs = mkpath2(orgfname,ETCDNAME,sip->ofn)) >= 0) {
	    if ((rs = readfileline(sip->rbuf,sip->rlen,orgfname)) >= 0) {
	        len = rs ;
	    } else if (isNotPresent(rs))
		rs = SR_OK ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (localgetorg_sys) */


static int subinfo_homer(SUBINFO *sip,const char *homeuser)
{
	int		rs ;
	int		len = 0 ;
	char		homedname[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("subinfo_homer: un=%s\n",homeuser) ;
#endif

	if ((rs = getuserhome(homedname,MAXPATHLEN,homeuser)) >= 0) {
	    char	orgname[MAXNAMELEN+1] ;
#if	CF_DEBUGS
	    debugprintf("subinfo_homer: userhome=%s\n",homedname) ;
#endif
	    if ((rs = sncpy2(orgname,MAXNAMELEN,"/.",sip->ofn)) >= 0) {
		char	orgfname[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(orgfname,homedname,orgname)) >= 0) {
		    const int	rlen = sip->rlen ;
		    char	*rbuf = sip->rbuf ;
	            if ((rs = readfileline(rbuf,rlen,orgfname)) >= 0) {
	                len = rs ;
	            } else if (isNotPresent(rs))
		        rs = SR_OK ;
		} /* end if (mkpath) */
	    } /* end if (sncpy) */
	} /* end if (getuserhome) */

#if	CF_DEBUGS
	debugprintf("subinfo_homer: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_homer) */


static int subinfo_passwder(SUBINFO *sip,const char *homeuser)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	int		len = 0 ;
	char		*pwbuf ;

	sip->rbuf[0] = '\0' ;
	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,homeuser)) >= 0) {
		GECOS	g ;
	        if ((rs = gecos_start(&g,pw.pw_gecos,-1)) >= 0) {
		    int		vl ;
		    const int	gi = gecosval_organization ;
		    const char	*vp ;
	            if ((vl = gecos_getval(&g,gi,&vp)) > 0) {
	    	        rs = sncpy1w(sip->rbuf,sip->rlen,vp,vl) ;
		        len = rs ;
		    }
	            gecos_finish(&g) ;
	        } /* end if (GECOS) */
	    } /* end if (get PW entry) */
	    uc_free(pwbuf) ;
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("subinfo_passwder: ret rs=%d len=%u\n",rs,len) ;
	debugprintf("subinfo_passwder: rbuf=>%t<\n",sip->rbuf,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_passwder) */


