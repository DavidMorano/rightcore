/* uc_atexit */

/* interface component for UNIX® library-3c */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Happily, unlike some (several) other middleware calls to the UNIX®
	system, this one is quite straight-forward.


*******************************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>		/* |atexit(3c)| */
#include	<errno.h>

#include	<vsystem.h>


/* typedefs */

typedef void	(*atexit_t)(void) ;


/* exported subroutines */


int uc_atexit(atexit_t func)
{
	int		rs = SR_OK ;
	if (func == NULL) return SR_FAULT ;
	if (atexit(func) < 0) rs = (- errno) ;
	return rs ;
}
/* end subroutine (uc_atexit) */


