/* uc_inetpton */

/* interface component for UNIX® library-3c */
/* convert a "print"able INET address to its binary form */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int uc_inetpton(int af,cchar *straddr,void *binaddr)
{
	int		rs = SR_OK ;
	int		rv ;

	if (straddr == NULL) return SR_FAULT ;
	if (binaddr == NULL) return SR_FAULT ;

	rv = inet_pton(af,straddr,binaddr) ;
	if (rv < 0) {
	    rs = (- errno) ;
	} else if (rv == 0)
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (uc_inetpton) */


