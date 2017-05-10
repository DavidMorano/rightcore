/* isleapyear */

/* is the given year a leap year? */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This subroutine determines if the supplied given year is a leap year.

	Synopsis:

	int isleapyear(yr)
	int 	yr ;

	Arguments:

	yr		current year (ex: 1998)

	Returns:

	==0		no
	>0		yes

	Notes:

	= Remember to account for leap-year:

define	isleap(y) ((((y) % 4) == 0) && (((y) % 100) != 0 || ((y) % 400) == 0))


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int isleapyear(int y)
{
	int		f = ((y % 4) == 0) ;
	f = f && (((y % 100) != 0) || ((y % 400) == 0)) ;
	return f ;
}
/* end subroutine (isleapyear) */


