/* strlen */

/* calculate the length of a string */


/* revision history:

	= 1982-09-10, David A­D­ Morano

	This subroutine was written because I need this on our own
	embedded (VMS CPU) platform.


*/

/* Copyright © 1992 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is a knock off of the 'strlen()' from the
	regular UNIX® system.


*******************************************************************************/


#include	<envatandards.h>
#include	<sys/types.h>
#include	<string.h>


/* external subroutines */


int strlen(s)
const char	*s ;
{
	int	len = 0 ;

	while (*s++) len += 1 ;

	return len ;
}
/* end subroutine (strlen) */


