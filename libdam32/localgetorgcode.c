/* localgetorgcode */

/* get the LOCAL organization-code (ORGCODE) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        This subroutine is originally written. This is a minimal implementation.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to (try to) get the LOCAL software distribution
        organization-code (ORGCODE).

	Synopsis:

	int localgetorgcode(pr,rbuf,rlen,un)
	const char	pr[] ;
	char		rbuf[] ;
	char		rlen ;
	const char	*un ;

	Arguments:

	pr		program root
	rbuf		caller supplied buffer to place result in
	rlen		length of caller supplied buffer
	un		username

	Returns:

	>=0		length of returned ID
	<0		error in process of creating ID


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARORGCODE
#define	VARORGCODE	"ORGCODE"
#endif

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif

#define	ETCDNAME	"etc"
#define	ORGCODEFNAME	"orgcode"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	getuserorg(char *,int,const char *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	readfileline(char *,int,const char *) ;
extern int	touc(int) ;
extern int	isNotPresent(int) ;


/* external variables */


/* local structures */


/* forward references */

static int	procorg(char *,int,const char *,int) ;


/* local variables */


/* exported subroutines */


int localgetorgcode(cchar *pr,char *rbuf,int rlen,cchar *un)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	const char	*etcdname = ETCDNAME ;
	const char	*ocfname = ORGCODEFNAME ;
	const char	*orgcode = getenv(VARORGCODE) ;
	char		tfname[MAXPATHLEN+1] ;

	if (pr == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	rbuf[0] = '\0' ;

/* user environment */

	if ((len <= 0) && ((rs >= 0) || isNotPresent(rs))) {
	    if ((orgcode != NULL) && (orgcode[0] != '\0')) {
	        rs = sncpy1(rbuf,rlen,orgcode) ;
	        len = rs ;
	    }
	}

/* user configuration */

	if ((len <= 0) && ((rs >= 0) || isNotPresent(rs))) {
	    const int	hlen = MAXPATHLEN ;
	    char	hbuf[MAXPATHLEN+1] ;
	    if ((un == NULL) || (un[0] == '\0')) un = "-" ;
	    if ((rs = getuserhome(hbuf,hlen,un)) >= 0) {
		if ((rs = mkpath3(tfname,hbuf,etcdname,ocfname)) >= 0) {
		    rs = readfileline(rbuf,rlen,tfname) ;
		    len = rs ;
		}
	    } /* end if (getuserhome) */
	}

/* software facility (LOCAL) configuration */

	if ((len <= 0) && ((rs >= 0) || isNotPresent(rs))) {
	    if ((rs = mkpath3(tfname,pr,etcdname,ocfname)) >= 0) {
	        rs = readfileline(rbuf,rlen,tfname) ;
	        len = rs ;
	    }
	}

/* any operating system configuration (in '/etc') */

	if ((len <= 0) && ((rs >= 0) || isNotPresent(rs))) {
	    if ((rs = mkpath3(tfname,"/",etcdname,ocfname)) >= 0) {
	        rs = readfileline(rbuf,rlen,tfname) ;
	        len = rs ;
	    }
	}

/* create it out of the abbreviation of the organization name */

	if ((len <= 0) && ((rs >= 0) || isNotPresent(rs))) {
	    const int	orglen = ORGLEN ;
	    char	orgbuf[ORGLEN+1] ;
	    if ((un == NULL) || (un[0] == '\0')) un = "-" ;
	    rs = getuserorg(orgbuf,orglen,un) ;
	    if ((rs == SR_NOENT) || (rs == 0)) {
	        rs = localgetorg(pr,orgbuf,orglen,un) ;
	    }
	    if (rs > 0) {
		rs = procorg(rbuf,rlen,orgbuf,rs) ;
		len = rs ;
	    }
	}

/* get out (nicely as possible in our case) */

	if ((rs < 0) && isNotPresent(rs)) {
	    rs = SR_OK ;
	    len = 0 ;
	}

#if	CF_DEBUGS
	debugprintf("localgetorgcode: ret rs=%d org=>%s<\n",rs,rbuf) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (localgetorgcode) */


/* local subroutines */


static int procorg(char *rbuf,int rlen,cchar *orgbuf,int orglen)
{
	int		rs = SR_OK ;
	int		cl ;
	int		len = 0 ;
	int		sl = orglen ;
	const char	*sp = orgbuf ;
	const char	*cp ;

	while (sl && ((cl = nextfield(sp,sl,&cp)) > 0)) {

	    if (len >= rlen) break ;
	    rbuf[len++] = touc(cp[0]) ;

	    sl -= ((cp+cl)-sp) ;
	    sp = (cp+cl) ;
	} /* end while */

	rbuf[len] = '\0' ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (progorg) */


