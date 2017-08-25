/* inetaddr */

/* object to manipulate INET adresses */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-14, David A­D­ Morano
	This little object module was first written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object allows for some common manipulations on INET4
	addresses.


*******************************************************************************/


#define	INETADDR_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ctdec.h>
#include	<char.h>
#include	<localmisc.h>

#include	"inetaddr.h"


/* local defines */

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(struct in_addr)
#endif

#define	BADTHING	(~ 0)


/* external subroutines */

extern int	cfnumui(char *,int,uint *) ;
extern int	getdig(int) ;

extern char	*strwcpy(char *,char *,int) ;


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int inetaddr_start(inetaddr *ip,void *addr)
{

	if (ip == NULL) return SR_FAULT ;
	if (addr == NULL) return SR_FAULT ;

	memcpy(&ip->a.s_addr,addr,INET4ADDRLEN) ;

	return SR_OK ;
}
/* end subroutine (inetaddr_start) */


int inetaddr_startstr(inetaddr *ip,char *addr,int addrlen)
{
	int		rs = SR_OK ;
	char		*ap = addr ;
	char		abuf[(INET4ADDRLEN * 4) + 1] ;

	if (ip == NULL) return SR_FAULT ;
	if (addr == NULL) return SR_FAULT ;

	if (addrlen < 0)
	    addrlen = strlen(addr) ;

	while (CHAR_ISWHITE(*ap)) {
	    ap += 1 ;
	    addrlen -= 1 ;
	}

	if (*ap == '\\') {
	    uint	uiw ;
	    rs = cfnumui(ap,addrlen,&uiw) ;
	    ip->a.s_addr = htonl(uiw) ;
	} else {
	    while ((addrlen > 0) && CHAR_ISWHITE(ap[addrlen - 1])) {
	        addrlen -= 1 ;
	    }
	    if (addrlen > 0) {
	        if (ap != addr) {
	            strwcpy(abuf,ap,addrlen) ;
	            ap = abuf ;
	        }
	        ip->a.s_addr = (in_addr_t) inet_addr(abuf) ;
	        if (ip->a.s_addr == ((in_addr_t) BADTHING)) {
	            rs = SR_INVALID ;
		}
	    } else {
	        rs = SR_INVALID ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (inetaddr_startstr) */


int inetaddr_startdot(inetaddr *ip,char *addr,int addrlen)
{
	int		rs = SR_OK ;
	char		*ap = addr ;
	char		abuf[(INET4ADDRLEN * 4) + 1] ;

	if (ip == NULL) return SR_FAULT ;
	if (addr == NULL) return SR_FAULT ;

	if (addrlen < 0)
	    addrlen = strlen(addr) ;

/* remove leading white space */

	while (CHAR_ISWHITE(*ap)) {
	    ap += 1 ;
	    addrlen -= 1 ;
	}

/* remove trailing white space */

	while ((addrlen > 0) && CHAR_ISWHITE(ap[addrlen-1])) {
	    addrlen -= 1 ;
	}

	if (addrlen > 0) {
	    if (ap != addr) {
	        strwcpy(abuf,ap,addrlen) ;
	        ap = abuf ;
	    }
	    ip->a.s_addr = (in_addr_t) inet_addr(abuf) ;
	    if (ip->a.s_addr == ((in_addr_t) BADTHING)) {
	        rs = SR_INVALID ;
	    }
	} else {
	    rs = SR_INVALID ;
	}

	return rs ;
}
/* end subroutine (inetaddr_startdot) */


int inetaddr_finish(inetaddr *ip)
{

	if (ip == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (inetaddr_finish) */


int inetaddr_gethexaddr(inetaddr *ip,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		j = 0 ;

	if (ip == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if ((rlen < 0) || (rlen >= ((INET4ADDRLEN * 2) + 0))) {
	    uint	v ;
	    int		i ;
	    for (i = 0 ; i < INET4ADDRLEN ; i += 1) {
	        v = MKCHAR(ip->straddr[i]) ;
	        rbuf[j++] = getdig((v >> 4) & 0xf) ;
	        rbuf[j++] = getdig((v >> 0) & 0xf) ;
	    } /* end for */
	} else {
	    rs = SR_OVERFLOW ;
	}

	rbuf[j] = '\0' ;
	return (rs >= 0) ? j : rs ;
}
/* end subroutine (inetaddr_gethexaddr) */


int inetaddr_getdotaddr(inetaddr *ip,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	char		*bp = rbuf ;

	if (ip == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if ((rlen < 0) || (rlen >= ((INET4ADDRLEN * 3) + 3))) {
	    uint	v ;
	    int		i ;
	    for (i = 0 ; i < INET4ADDRLEN ; i += 1) {
	        if (i > 0) *bp++ = '.' ;
	        v = MKCHAR(ip->straddr[i]) ;
	        rs = ctdecui(bp,3,v) ;
	        bp += rs ;
	        if (rs < 0) break ;
	    } /* end for */
	} else {
	    rs = SR_OVERFLOW ;
	}

	*bp = '\0' ;
	return (rs >= 0) ? (bp - rbuf) : rs ;
}
/* end subroutine (inetaddr_getdotaddr) */


