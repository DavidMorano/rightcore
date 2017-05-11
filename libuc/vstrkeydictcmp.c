/* vstrkeydictcmp */

/* vector string key comparison */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine makes a comparison of the key of a string that looks
        like a SHELL variable assignment.

	For example, consider the following compound string :

		A=the_dog_house

        The 'A' would be the key, and the part 'the_dog_house' is the value.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	strkeydictcmp(const char *,const char *) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


/* compare routine for environment variable search */
int vstrkeydictcmp(e1p,e2p)
const char	**e1p, **e2p ;
{
	int	rc ;


	if ((*e1p == NULL) && (*e2p == NULL))
	    return 0 ;

	if (*e1p == NULL)
	    return 1 ;

	if (*e2p == NULL)
	    return -1 ;

	rc = strkeydictcmp(*e1p,*e2p) ;

	return rc ;
}
/* end subroutine (vstrkeydictcmp) */


