/* iseol */

/* is the character an EOL character? */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Return whether the given character is part of the EOL sequence.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int iseol(int ch)
{
	int		f = FALSE ;
	f = f || (ch == '\n') ;
	f = f || (ch == '\r') ;
	return f ;
}
/* end subroutine (iseol) */


int isEOL(int ch)
{
	return iseol(ch) ;
}
/* end subroutine (isEOL) */


