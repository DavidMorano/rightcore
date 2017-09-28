/* getaflen */

/* get the length of a socket address based on its address-family */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2005-02-03, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine divines the length of a socket address from an
        address-family.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(in_addr_t)
#endif

#ifndef	INET6ADDRLEN
#define	INET6ADDRLEN	16
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	inetpton(char *,int,int,const char *,int) ;
extern int	inetntop(char *,int,int,const void *) ;
extern int	isinetaddr(const char *) ;
extern int	isindomain(const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getaflen(int af)
{
	int	alen = SR_AFNOSUPPORT ;

	switch (af) {
	case AF_UNIX:
	    alen = MAXPATHLEN ;
	    break ;
	case AF_INET4:
	    alen = INET4ADDRLEN ;
	    break ;
	case AF_INET6:
	    alen = INET6ADDRLEN ;
	    break ;
	} /* end switch */

	return alen ;
}
/* end subroutine (getaflen) */


int getaddrlen(int af)
{
	return getaflen(af) ;
}
/* end subroutine (getaddrlen) */


