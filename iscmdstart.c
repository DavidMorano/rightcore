/* iscmdstart */

/* is the given character the (possible) start of a terminal command? */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks if the given (passed) character could be the
        start of a terminal command.

	Synopsis:

	int iscmdstart(ch)
	int	ch ;

	Arguments:

	ch		character to test

	Returns:

	0		no
	1		yes


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<ascii.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int iscmdstart(int ch)
{
	int		f = FALSE ;
	f = f || (ch == CH_ESC) ;
	f = f || (ch == CH_CSI) ;
	f = f || (ch == CH_DCS) ;
	f = f || (ch == CH_SS3) ;
	return f ;
}
/* end subroutine (iscmdstart) */


