/* isprintbad */

/* is the character not printable? */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Determine if a character is bad to print.

	Synopsis:

	int isprintbad(ch)
	int	ch ;

	Arguments:

	ch		character to check

	Returns:

	TRUE		character is not printable
	FALSE		character is printable


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* forward references */


/* external subroutines */

extern int	isprintlatin(int) ;


/* exported subroutines */


int isprintbad(int ch)
{
	return (! isprintlatin(ch)) ;
}
/* end subroutine (isprintbad) */


