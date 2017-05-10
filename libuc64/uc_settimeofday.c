/* uc_settimeofday */

/* interface component for UNIX® library-3c */
/* set the current time of day */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This routine calls the system's (library) 'settimeofday' subroutine.

	Question:
		What does this stupid function return?
		The documentation is not as clear as it could be!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/time.h>
#include	<unistd.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */


int uc_settimeofday(tvp,dp)
struct timeval	*tvp ;
void		*dp ;
{
	int	rs ;


	if (tvp == NULL)
		return SR_FAULT ;

	rs = SR_OK ;
	if (settimeofday(tvp,dp) == -1)
		rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_settimeofday) */


