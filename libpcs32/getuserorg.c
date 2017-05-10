/* getuserorg */

/* get the organization name (string) for a specified user-name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_ORGSYS	0		/* get from system? */
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

	int getuserorg(rbuf,rlen,username)
	char		rbuf[] ;
	int		rlen ;
	const char	*username ;

	Arguments:

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

#ifndef	ETCDNAME
#define	ETCDNAME	"/etc"
#endif

#undef	ORGCNAME
#define	ORGCNAME	"organization"

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif

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
extern int	getuserhome(char *,int,const char *) ;
extern int	readfileline(char *,int,const char *) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct subinfo {
	const char	*ofp ;
	const char	*un ;
	char		*rbuf ;		/* user-supplied buffer */
	int		rlen ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,const char *,char *,int,const char *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	getuserorg_var(SUBINFO *) ;
static int	getuserorg_home(SUBINFO *) ;
static int	getuserorg_passwd(SUBINFO *) ;
#if	CF_ORGSYS
static int	getuserorg_sys(SUBINFO *) ;
#endif

static int	isNoAcc(int) ;


/* local variables */

static const int	rsnoacc[] = {
	SR_NOENT,
	SR_ACCESS,
	0
} ;


/* exported subroutines */


int getuserorg(rbuf,rlen,username)
char		rbuf[] ;
int		rlen ;
const char	*username ;
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		len = 0 ;
	const char	*ofp = ORGCNAME ;

	if (rbuf == NULL) return SR_FAULT ;
	if (username == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if (username[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("getuserorg: u=%s\n",username) ;
#endif

	if ((rs = subinfo_start(&si,ofp,rbuf,rlen,username)) >= 0) {
	    int	i ;

	    for (i = 0 ; i < 3 ; i += 1) {
	        switch (i) {
	        case 0:
	            rs = getuserorg_var(sip) ;
	            break ;
	        case 1:
	            rs = getuserorg_home(sip) ;
	            break ;
	        case 2:
	            rs = getuserorg_passwd(sip) ;
	            break ;
	        } /* end switch */
	        len = rs ;

#if	CF_DEBUGS
	        debugprintf("getuserorg: getuserorg_%u() rs=%d\n",i,rs) ;
#endif

	        if (rs != 0) break ;
	    } /* end for */

	    subinfo_finish(&si) ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("getuserorg: ret rs=%d len=%u\n",rs,len) ;
	debugprintf("getuserorg: org=>%s<\n",rbuf) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getuserorg) */


int gethomeorg(rbuf,rlen,homedname)
char		rbuf[] ;
int		rlen ;
const char	*homedname ;
{
	int		rs ;
	int		len = 0 ;
	const char	*orgcname = ORGCNAME ;
	char		cname[MAXNAMELEN+1] ;

	if (rbuf == NULL) return SR_FAULT ;
	if (homedname == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if (homedname[0] == '\0') return SR_INVALID ;

	if ((rs = sncpy2(cname,MAXNAMELEN,".",orgcname)) >= 0) {
	    char	orgfname[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(orgfname,homedname,cname)) >= 0) {
	        if ((rs = readfileline(rbuf,rlen,orgfname)) >= 0) {
	            len = rs ;
		} else if (isNoAcc(rs))
		    rs = SR_OK ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (gethomeorg) */


/* local subroutines */


static int subinfo_start(sip,ofp,rbuf,rlen,un)
SUBINFO		*sip ;
const char	*ofp ;
char		*rbuf ;
int		rlen ;
const char	*un ;
{

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->ofp = ofp ;
	sip->rbuf = rbuf ;
	sip->rlen = rlen ;
	sip->un = un ;

	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
SUBINFO		*sip ;
{

	if (sip == NULL)
	    return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int getuserorg_var(SUBINFO *sip)
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
/* end subroutine (getuserorg_var) */


static int getuserorg_home(SUBINFO *sip)
{
	int		rs ;
	int		len = 0 ;
	char		homedname[MAXPATHLEN+1] ;

	if ((rs = getuserhome(homedname,MAXPATHLEN,sip->un)) >= 0) {
	    char	orgname[MAXNAMELEN+1] ;
	    if ((rs = sncpy2(orgname,MAXNAMELEN,"/.",sip->ofp)) >= 0) {
	        char	ofname[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(ofname,homedname,orgname)) >= 0) {
	            if ((rs = readfileline(sip->rbuf,sip->rlen,ofname)) >= 0) {
	                len = rs ;
		    } else if (isNoAcc(rs))
			rs = SR_OK ;
		}
	    }
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getuserorg_home) */


static int getuserorg_passwd(SUBINFO *sip)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs = SR_OK ;
	int		len = 0 ;
	char		*pwbuf ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    if (sip->un[0] == '-') {
	        rs = getpwusername(&pw,pwbuf,pwlen,-1) ;
	    } else {
	        rs = GETPW_NAME(&pw,pwbuf,pwlen,sip->un) ;
	    }
	    if (rs >= 0) {
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

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getuserorg_passwd) */


#if	CF_ORGSYS

static int getuserorg_sys(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	char		orgfname[MAXPATHLEN+1] ;

	if ((rs = mkpath2(orgfname,ETCDNAME,sip->ofp)) >= 0) {
	    rs = readfileline(sip->rbuf,sip->rlen,orgfname) ;
	    len = rs ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getuserorg_sys) */

#endif /* CF_ORGSYS */


static int isNoAcc(int rs)
{
	return isOneOf(rsnoacc,rs) ;
}
/* end subroutine (isNoAcc) */


