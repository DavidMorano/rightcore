/* isprintterm */

/* is a character printable for a terminal? */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is sort of like 'isprint(3c)' but allows for ISO Latin-1
        characters also.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<ascii.h>


/* external subroutines */

extern int	isprintlatin(int) ;


/* exported subroutines */


int isprintterm(int ch)
{
	int		f = isprintlatin(ch) ;

	f = f || (ch == CH_BEL) ;
	f = f || (ch == CH_TAB) ;
	f = f || (ch == CH_BS) ;
	f = f || (ch == CH_CR) ;
	f = f || (ch == CH_NL) ;
	f = f || (ch == '\v') ;
	f = f || (ch == '\f') ;
	f = f || (ch == CH_SO) || (ch == CH_SI) ;
	f = f || (ch == CH_SS2) || (ch == CH_SS3) ;

	return f ;
}
/* end subroutine (isprintterm) */


