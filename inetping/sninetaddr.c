/* sninetaddr */

/* make string version of INET addresses */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_CTHEXUC	1		/* use 'cthexuc(3dam)' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Create and copy a string repreentation of an Internet address (either
	IP4 or IP5) into the destination.

	Synopsis:

	int sninetaddr(dbuf,dlen,af,addr)
	char		*dbuf ;
	int		dlen ;
	int		af ;
	const char	*addr ;
	
	Arguments:

	dbuf		destination string buffer
	dlen		destination string buffer length
	af		address-family
	addr		address buffer

	Returns:

	>=0		number of bytes in result
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netinet/in.h>
#include	<string.h>

#include	<vsystem.h>
#include	<inetaddr.h>
#include	<cthex.h>
#include	<localmisc.h>


/* local defines */

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(in_addr_t)
#endif

#ifndef	INET6ADDRLEN
#define	INET6ADDRLEN	16
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	MAX(INET4ADDRLEN,INET6ADDRLEN)
#endif /* INETXADDRLEN */

#ifndef	INET4_ADDRSTRLEN
#define	INET4_ADDRSTRLEN	16
#endif

#ifndef	INET6_ADDRSTRLEN
#define	INET6_ADDRSTRLEN	46	/* Solaris® says this is 46! */
#endif

#ifndef	INETX_ADDRSTRLEN
#define	INETX_ADDRSTRLEN	MAX(INET4_ADDRSTRLEN,INET6_ADDRSTRLEN)
#endif

#define	DBUFLEN		10


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;

extern char	*strdcpy1(char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int snunix(char *,int,const char *) ;
static int sninet4(char *,int,const char *) ;
static int sninet6(char *,int,const char *) ;


/* local variables */


/* exported subroutines */


int sninetaddr(char dbuf[],int dlen,int af,cchar *addr)
{
	int		rs = SR_OK ;

	if (dbuf == NULL) return SR_FAULT ;
	if (addr == NULL) return SR_FAULT ;

	switch (af) {
	case AF_UNIX:
	    rs = snunix(dbuf,dlen,addr) ;
	    break ;
	case AF_INET4:
	    rs = sninet4(dbuf,dlen,addr) ;
	    break ;
	case AF_INET6:
	    rs = sninet6(dbuf,dlen,addr) ;
	    break ;
	default:
	    rs = SR_AFNOSUPPORT	;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (sninetaddr) */


/* local subroutines */


static int snunix(char *dbuf,int dlen,const char *addr)
{
	int		rs ;

#ifdef	COMMENT
	rs = strdcpy1(dbuf,dlen,addr) - dbuf ;
#else /* COMMENT */
	rs = sncpy1(dbuf,dlen,addr) ;
#endif /* COMMENT */

	return rs ;
}
/* end subroutine (snunix) */


static int sninet4(char *dbuf,int dlen,cchar *addr)
{
	INETADDR	ia ;
	int		rs ;

	if ((rs = inetaddr_start(&ia,addr)) >= 0) {
	    {
	        rs = inetaddr_getdotaddr(&ia,dbuf,dlen) ;
	    }
	    inetaddr_finish(&ia) ;
	} /* end if (inetaddr) */

	return rs ;
}
/* end subroutine (sninet4) */


static int sninet6(char *dbuf,int dlen,cchar *addr)
{
	int		rs = SR_OK ;
	int		pl = 0 ;

	if ((dlen < 0) || (dlen >= INETX_ADDRSTRLEN)) {
	    uint	uch ;
	    const int	diglen = DBUFLEN ;
	    int		i ;
	    char	digbuf[DBUFLEN+1] ;
	    for (i = 0 ; (rs >= 0) && (i < INET6ADDRLEN) ; i += 1) {
		uch = MKCHAR(addr[i]) ;
#if	CF_CTHEXC
	        rs = cthexuc(digbuf,diglen,uch) ; /* cannot fail! */
	        if ((i > 0) && ((i & 1) == 0)) dbuf[pl++] = ':' ;
	        dbuf[pl++] = digbuf[0] ;
	        dbuf[pl++] = digbuf[1] ;
#else /* CF_CTHEXC */
	        rs = cthexui(digbuf,diglen,uch) ; /* cannot fail! */
	        if ((i > 0) && ((i & 1) == 0)) dbuf[pl++] = ':' ;
	        dbuf[pl++] = digbuf[6] ;
	        dbuf[pl++] = digbuf[7] ;
#endif /* CF_CTHEXC */
	    } /* end for */
	} else {
	    rs = SR_OVERFLOW ;
	}

	dbuf[pl] = '\0' ;
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (sninet6) */


