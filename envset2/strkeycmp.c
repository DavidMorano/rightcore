/* strkeycmp */

/* string key comparison */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine makes a comparison of the key of a string that looks
        like a SHELL variable assignment.

	For example, consider the following compound string:

		A=the_dog_house

	The 'A' would be the key, and the part 'the_dog_house' is the value.

	Synopsis:

	int strkeycmp(e1p,e2p)
	const char	*e1p, *e2p ;

	Arguments:

	e1p		first string
	e2p		second string

	Returns:

	>0		the second key is greater than the first
	0		the keys of the strings are equal
	<0		the first key is less than the second


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int strkeycmp(cchar *e1p,cchar *e2p)
{
	int		rc = 0 ;
	if ((e1p != NULL) || (e2p != NULL)) {
	    if (e1p != NULL) {
	        if (e2p != NULL) {
	            while (*e1p && *e2p) {
	                if ((*e1p == '=') || (*e2p == '=')) break ;
	                if (*e1p != *e2p) break ;
	                e1p += 1 ;
	                e2p += 1 ;
	            } /* end while */
	            if (*e1p != *e2p) {
	                if (*e1p != '=') {
	                    if (*e2p != '=') {
	                        rc = (*e1p - *e2p) ;
	                    } else
	                        rc = (*e1p == '\0') ? 0 : 1 ;
	                } else
	                    rc = (*e2p == '\0') ? 0 : -1 ;
	            }
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (strkeycmp) */


