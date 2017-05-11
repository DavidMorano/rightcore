/* calstrs */

/* calendar strings */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We provide a database of various common calendar strings.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */

cchar	*months[] = {
	"January", "February", "March", "April", "May", "June", "July", 
	"August", "September", "October", "November", "December", NULL
} ;

cchar	*days[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", 
	"Friday", "Saturday", NULL
} ;


/* exported subroutines */


