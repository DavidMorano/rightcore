/* isclass */

/* is a character one of a certain character class? */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano
	The subroutine set was written from scratch to do what the
	previous program by the same name did.

*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines check a character to see if it is part of a special
        character class. See the code for details!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"contentencodings.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* exported subroutines */


/* is it 7-bit text (no controls or other weirdo) */
int is7bit(uint c)
{
	return (isascii(c) && isprint(c)) ;
}
/* end subroutine (is7bit) */


/* is it 8-bit text (no controls or other weirdo) */
int is8bit(uint c)
{
	return ((c & 0x80) && ((c & (~ 31)) != 0x80)) ;
}
/* end subroutine (is8bit) */


int isbinary(uint c)
{
	int	f = FALSE ;
	if (! isspace(c)) {
	    f = (((c & (~ 31)) == 0x00) || ((c & (~ 31)) == 0x80)) ;
	}
	return f ;
}
/* end subroutine (isbinary) */


