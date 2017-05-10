/* uc_reuseaddr */

/* interface component for UNIX® library-3c */
/* reuse an existing (perhaps) socket address */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/socket.h>


/* exported subroutines */


int uc_reuseaddr(int s)
{
	const int	optlen = sizeof(int) ;
	int		rs ;
	int		one = 1 ;

	rs = u_setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,optlen) ;

	return rs ;
}
/* end subroutine (uc_reuseaddr) */


