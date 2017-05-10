/* vstrkeycmp */

/* vector string key comparison */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine makes a comparison of the key of a string that 
	looks like a SHELL variable assignment.

	For example, consider the following compound string :

		A=the_dog_house

	The 'A' would be the key, and the part 'the_dog_house' is
	the value.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	strkeycmp(const char *,const char *) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int vstrkeycmp(s1p,s2p)
const char	**s1p, **s2p ;
{
	int	rc ;

	const char	*s1 = (const char *) *s1p ;
	const char	*s2 = (const char *) *s2p ;


	if ((s1 == NULL) && (s2 == NULL)) 
		return 0 ;

	if (s1 == NULL) 
		return 1 ;

	if (s2 == NULL) 
		return -1 ;

	rc = (*s1 - *s2) ;
	if (rc == 0)
	    rc = strkeycmp(s1,s2) ;

	return rc ;
}
/* end subroutine (vstrkeycmp) */



