/* isdigitlatin */

/* is the specified character a digit? */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is sort of like 'isdigit(3c)' but allows for ISO
	Latin-1 characters also.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* external subroutines */


/* forward references */


/* local variables */


/* exported subroutines */


int isdigitlatin(int ch)
{
	return (ch >= '0') && (ch <= '9') ;
}
/* end subroutine (isdigitlatin) */


