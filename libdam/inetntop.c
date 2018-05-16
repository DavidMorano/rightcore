/* inetntop */

/* convert binary INET addresses to portable string representation */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2003-11-06, David A­D­ Morano
        This subroutine converts an INET network-order object to a portable
	string representation.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutines converts an INET network-order object (binary) to a 
	portable string representation.

	Synopsis:

	int inetntop(rbuf,rlen,af,binaddr)
	char		rbuf[] ;
	int		rlen ;
	int		af ;
	const void	*binaddr ;

	Arguments:

	rbuf		buffer to receive result
	rlen		length of buffer to receive result
	af		address-family (AF) of source address
	str		string representing the source address

	Returns:

	>=0		length of resulting string
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local variables */


/* exported subroutines */


int inetntop(char *rbuf,int rlen,int af,const void *binaddr)
{
	int		rs ;

	if (rbuf == NULL) return SR_FAULT ;
	if (binaddr == NULL) return SR_FAULT ;

	rs = uc_inetntop(af,binaddr,rbuf,rlen) ;

	if (rs == SR_NOSPC) rs = SR_OVERFLOW ; /* fix mistake in standard */

	return rs ;
}
/* end subroutine (inetntop) */


