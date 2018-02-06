/* sigdefaults */

/* set a given set of signals to their default disposition */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine sets the disposition of the specified set of signals
	(given as an array of integers containing the desired signal numbers)
	to their default condition.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>


/* external subroutines */


/* external variables */


/* local strutures */


/* forward references */


/* local variables */


/* exported subroutines */


int sigdefaults(int *sigs)
{
	int		rs = SR_OK ;

	if (sigs != NULL) {
	    int	i ;
	    for (i = 0 ; (rs >= 0) && (sigs[i] > 0) ; i += 1) {
		rs = uc_sigdefault(sigs[i]) ;
	    }
	} else {
	    rs = SR_FAULT ;
	}

	return rs ;
}
/* end subroutine (sigdefaults) */


