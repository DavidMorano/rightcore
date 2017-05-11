/* uc_atexit */

/* interface component for UNIX® library-3c */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_atexit(void (*func)())
{
	int	rs = SR_OK ;

	if (func == NULL) return SR_FAULT ;

	if (atexit(func) < 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_atexit) */


