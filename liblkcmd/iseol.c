/* iseol */

/* is the character an EOL (End-Of-Line) character? */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Return whether the given character constitutes an EOL (End-Of-Line)
	character.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int iseol(int ch)
{
	return ((ch == '\n') || (ch == '\r')) ;
}
/* end subroutine (iseol) */


int isEOL(int ch)
{
	return iseol(ch) ;
}
/* end subroutine (isEOL) */


