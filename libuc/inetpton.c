/* inetpton */

/* convert strings to binary INET addresses */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2003-11-06, David A­D­ Morano
        This subroutine exists to make some sensible version out of the
        combination of 'inet_addr(3n)' and 'inet_pton(3socket)'.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutines converts a string representation of an INET address
	(either v4 or v6) into its binary form.

	Synopsis:

	int inetpton(addrbuf,addrlen,af,sp,sl)
	void		*addrbuf ;
	int		addrlen ;
	int		af ;
	const char	sp[] ;
	int		sl ;

	Arguments:

	addrbuf		buffer to receive result
	addrlen		length of buffer to receive result
	af		address-family (AF) of source address
	sp		string representing the source address
	sl		length of the string representing the source address

	Returns:

	>=0		the address family of the resulting address
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(in_addr_t)
#endif

#ifndef	INET6ADDRLEN
#define	INET6ADDRLEN	sizeof(in6_addr_t)
#endif

#ifndef	INET4_ADDRSTRLEN
#define	INET4_ADDRSTRLEN	16
#endif

#ifndef	INET6_ADDRSTRLEN
#define	INET6_ADDRSTRLEN	46	/* Solaris® says this is 46! */
#endif

#define	ASTRLEN		MAX(INET4_ADDRSTRLEN,INET6_ADDRSTRLEN)

#define	BADTHING	((uint) (~ 0))


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* local variables */


/* exported subroutines */


int inetpton(void *addrbuf,int addrlen,int af,cchar *srcbuf,int srclen)
{
	int		rs ;		/* not necessary to initialize */
	int		sl ;
	const char	*sp ;

#if	CF_DEBUGS
	debugprintf("inetpton: ent af=%d src=%t\n",af,srcbuf,srclen) ;
	debugprintf("inetpton: addrlen=%d\n",addrlen) ;
#endif

	if (addrbuf == NULL) return SR_FAULT ;
	if (srcbuf == NULL) return SR_FAULT ;

	if (af < 0) return SR_INVALID ;

	if ((sl = sfshrink(srcbuf,srclen,&sp)) > 0) {
	    char	astr[ASTRLEN + 1] ;

#if	CF_DEBUGS
	    debugprintf("inetpton: af=%d src=%t\n",af,sp,sl) ;
#endif

	    if (af == AF_UNSPEC) {
	        af = (strnchr(sp,sl,':') != NULL) ? AF_INET6 : AF_INET4 ;
	    }

	    if (af == AF_INET4) {

	        if (addrlen >= INET4ADDRLEN) {
	            in_addr_t	a ;

	            strwcpy(astr,sp,MIN(sl,ASTRLEN)) ;

	            a = inet_addr(astr) ;

	            if (a != BADTHING) {
			rs = SR_OK ;
	                memcpy(addrbuf,&a,INET4ADDRLEN) ;
		    } else
			rs = SR_INVALID ;

	        } else
	            rs = SR_OVERFLOW ;

	    } else if (af == AF_INET6) {

	        if (addrlen >= INET6ADDRLEN) {

	            strwcpy(astr,sp,MIN(sl,ASTRLEN)) ;

	            rs = uc_inetpton(af,addrbuf,astr) ;

	        } else
	            rs = SR_OVERFLOW ;

	    } else
	        rs = SR_AFNOSUPPORT ;

	} else
	    rs = SR_DOM ;

#if	CF_DEBUGS
	debugprintf("inetpton: ret rs=%d af=%u\n",rs,af) ;
#endif

	return (rs >= 0) ? af : rs ;
}
/* end subroutine (inetpton) */


