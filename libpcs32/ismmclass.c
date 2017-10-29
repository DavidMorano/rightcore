/* ismmclass */

/* is a character one of a certain character class? */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine set was written from scratch to do what the previous
        program by the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines check a character to see if it is part of a special
        character class. See the code for details!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"contentencodings.h"


/* local defines */


/* external subroutines */

extern int	isprintlatin(int) ;


/* external variables */


/* local structures */


/* forward references */


/* exported subroutines */


/* is it 7-bit text (no controls or other weirdo) */
int ismmclass_7bit(int ch)
{
	int		f = FALSE ;
	ch &= UCHAR_MAX ;
	if (ch < 128) {
	    f = f || isprintlatin(ch) ;
	    f = f || (ch == '\n') ;
	    f = f || (ch == '\r') ;
	    f = f || (ch == CH_TAB) ;
	    f = f || (ch == CH_SP) ;
	}
	return f ;
}
/* end subroutine (ismmclass_7bit) */


/* does it *require* 8-bit but *only* 8-bit */
int ismmclass_8bit(int ch)
{
	int		f = FALSE ;
	ch &= UCHAR_MAX ;
	if ((ch >= 128) && (ch < 256)) {
	    f = ((ch & 0x7f) >= 0x20) ;
	}
	return f ;
}
/* end subroutine (ismmclass_8bit) */


/* does it *require* binary */
int ismmclass_binary(uint ch)
{
	int		f = FALSE ;
	ch &= UCHAR_MAX ;
	if (((ch & 0x7f) < 0x20) || (ch == CH_DEL)) {
	    f = TRUE ;
	    f = f && (ch != '\n') ;
	    f = f && (ch != '\r') ;
	    f = f && (ch != CH_TAB) ;
	}
	return f ;
}
/* end subroutine (ismmclass_binary) */


