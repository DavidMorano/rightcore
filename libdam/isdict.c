/* isdict */

/* is a dictionary significant character (in the Latin-1 set) */


/* revision history:

	= 2007-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2007 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is similar to the 'isalnum(3c)' subroutine except that
	it checks if characters are "dictionary" significant.  Dictionary
	significant characters are:

		letters
		digits

	Synopsis:

	int isdict(ch)
	int	ch ;

	Arguments:

	ch	character to test

	Returns:

	FALSE	character is not a dictionary-significant character
	TRUE	character is a dictionary-significant character


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>		/* *required* */

#include	<ascii.h>
#include	<localmisc.h>


/* external subroutines */

extern int	isalphalatin(int) ;
extern int	isalnumlatin(int) ;


/* forward references */


/* external subroutines */


int isdict(int ch)
{
	int		f = FALSE ;
	if ((ch >= 0) && (ch < 0x100)) {
	    f = (ch < 0x80) && isalnum(ch) ;
	    f = f || ((ch >= 0xC0) && (ch != 0xD7) && (ch != 0xF7)) ;
	}
	return f ;
}
/* end subroutine (isdict) */


