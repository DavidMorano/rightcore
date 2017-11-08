/* getournetname */

/* get the RPC net-name for a user */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little diddy retrieves the RPC "netname" of the caller.  We first
	check whether the NIS server is running and if so, then perform the
        retrieval. For extra utility, we will also create an RPC net-name for a
        user other than ourselves. In the case of getting an RPC net-name for
        another user, we assume the name is correct. We do not really know if it
        is registered with the NIS server.

	Synopsis:

	int getournetname(nbuf,nlen,un)

	Arguments:

	nbuf		result buffer
	nlen		lenght of result buffer
	un		username

	Returns:

	<0		error
	>=0		length of result


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	PROCNAME	"keyserv"

#ifndef	MAXNETNAMELEN
#define	MAXNETNAMELEN	255	/* maximum length of network user's name */
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* exported subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern int	sncpy4w(char *,int,cchar *,cchar *,cchar *,cchar *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	nisdomainname(char *,int) ;
extern int	getuid_user(cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* forward references */

static int getothernetname(char *,int,cchar *) ;


/* exported subroutines */


int getournetname(char *nbuf,int nlen,cchar *un)
{
	int		rs ;
	int		len = 0 ;

	if (nbuf == NULL) return SR_FAULT ;

	if (nlen < 0) nlen = MAXNETNAMELEN ;

#if	CF_DEBUGS
	debugprintf("getournetname: ent nlen=%d un=%s\n",nlen,un) ;
#endif

	if ((un == NULL) || (un[0] == '\0') || (un[0] == '-')) {
	    if (nlen >= MAXNETNAMELEN) {
	        rs = uc_getnetname(nbuf) ;
	        len = rs ;
#if	CF_DEBUGS
		debugprintf("getournetname: uc_getnetnme() rs=%d\n",rs) ;
#endif
	    }  else {
	        char	netname[MAXNETNAMELEN+1] ;
	        if ((rs = uc_getnetname(netname)) >= 0) {
	            rs = sncpy1(nbuf,nlen,netname) ;
	            len = rs ;
	        }
	    }
	} else {
	    rs = getothernetname(nbuf,nlen,un) ;
	    len = rs ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("getournetname: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getournetname) */


/* local subroutines */


static int getothernetname(char *nbuf,int nlen,cchar *un)
{
	int		rs ;
	int		len = 0 ;
	cchar		*procname = PROCNAME ;
#if	CF_DEBUGS
	debugprintf("getotheretname: ent nlen=%d un=%s\n",nlen,un) ;
#endif
	if ((rs = uc_procpid(procname,0)) > 0) {
	    const int	dlen = MAXHOSTNAMELEN ;
	    char	dbuf[MAXHOSTNAMELEN+1] ;
#if	CF_DEBUGS
	    debugprintf("getotheretname: uc_procpid() rs=%d\n",rs) ;
#endif
	    if ((rs = nisdomainname(dbuf,dlen)) >= 0) {
	        const int	dl = rs ;
#if	CF_DEBUGS
	        debugprintf("getotheretname: nisdomainname() rs=%d\n",rs) ;
#endif
		if ((rs = getuid_user(un,-1)) >= 0) {
	            const int	dilen = DIGBUFLEN ;
	            const int	v = rs ;
	            char	dibuf[DIGBUFLEN+1] ;
	            if ((rs = ctdeci(dibuf,dilen,v)) >= 0) {
			cchar	*u = "unix." ;
	                rs = sncpy4w(nbuf,nlen,u,dibuf,"@",dbuf,dl) ;
	                len = rs ;
	            }
	        } /* end if (getuid_user) */
	    } /* end if (nisdomainname) */
	} else if (rs == 0) {
	    rs = SR_UNAVAIL ;
	}
#if	CF_DEBUGS
	debugprintf("getotheretname: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getothernetname) */


