/* isclass */

/* character classes for mail content purposes */


/* revision history:

	= 1996-03-01, David A­D­ Morano
        The subroutine set was written from scratch to do what the previous
        program by the same name did.

*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        These subroutines check a character to see if it is part of a special
        character class. See the code for details!


******************************************************************************/


#ifndef	ISCLASS_INCLUDE
#define	ISCLASS_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* exported subroutines */


#ifdef	__cplusplus
extern "C" {
#endif

extern int is7bit(int) ;
extern int is8bit(int) ;
extern int isbinary(int) ;

#ifdef	__cplusplus
}
#endif


#endif /* ISCLASS_INCLUDE */



