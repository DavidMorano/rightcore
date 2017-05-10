/* getnodename */

/* get the node-name of this node */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_GETHOSTNAME	0		/* use |gethostname(3c)| */
#define	CF_UNAME	0		/* allow 'u_uname(3u)' */
#define	CF_UINFO	1		/* use |uinfo(3uc)| */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine is an extraction of the corresponding code in the
	'getnodedomain(3dam)' subroutine.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine retrieves the node-name of the current node.

	Synopsis:

	int getnodename(rbuf,rlen)
	char		rbuf[] ;
	int		rlen ;

	Arguments:

	rbuf		buffer to receive the nodename 
	rlen		length of supplied buffer (should be NODENAMELEN)

	Returns:

	>=0		length of retrieved nodename
	<0		could not get the nodename (should be pretty rare!)

	Notes:

        The compiler person can customize how we find the current system
        nodename. Check out the compile-time defines at the top of this file. If
        you have it, generally (we think) using UINFO is probably best, because
        the result is process-wide cached. Nevertheless, we always (as it is
        now) use SI_HOSTNAME right now as a first resort (if the system supports
        that).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/systeminfo.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<uinfo.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARNODE
#define	VARNODE		"NODE"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy4(char *,int, const char *,const char *,
			const char *,const char *) ;
extern int	snwcpylc(const char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfwhitedot(const char *,int,const char **) ;

extern char	*strnchr(const char *,int,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getnodename(char *nbuf,int nlen)
{
	const int	hlen = NODENAMELEN ;
#if	CF_UNAME
	struct utsname	un ;
#endif
	int		rs = SR_OK ;
	int		sl = -1 ;
	const char	*sp = getenv(VARNODE) ;
	char		hbuf[NODENAMELEN + 1] ;

	if (nbuf == NULL)
	    return SR_FAULT ;

	nbuf[0] = '\0' ;

#if	defined(SI_HOSTNAME)
	if (sp == NULL) {
	    if ((sl = u_sysinfo(SI_HOSTNAME,hbuf,hlen)) >= 0)
		sp = hbuf ;
	}
#endif /* SI_HOSTNAME */

#if	CF_GETHOSTNAME
	if (sp == NULL) {
	    if ((sl = uc_gethostname(hbuf,hlen)) >= 0) {
		sp = hbuf ;
	    }
	} /* end if (trying GETHOSTNAME) */
#endif /* CF_GETHOSTNAME */

#if	CF_UINFO
	if (sp == NULL) {
	    UINFO_NAME	uin ;
	    if ((sl = uinfo_name(&uin)) >= 0) {
		sp = uin.nodename ; /* data does not go out of scope */
	    }
	} /* end if (trying GETHOSTNAME) */
#endif /* CF_UINFO */

#if	CF_UNAME
	if (sp == NULL) {
	    if ((rs = u_uname(&un)) >= 0) {
		sp = un.nodename ;
		sl = -1 ;
	    }
	} /* end if (trying UNAME) */
#endif /* CF_UNAME */

	if ((rs >= 0) && (sp != NULL)) {
	    int		cl ;
	    const char	*cp ;
	    rs = 0 ;
	    if ((cl = sfwhitedot(sp,sl,&cp)) > 0)
	        rs = snwcpy(nbuf,nlen,cp,cl) ;
	}

	return rs ;
}
/* end subroutine (getnodename) */


