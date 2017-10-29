/* strnkeycmp */

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

	int strnkeycmp(e1p,e2p,n)
	const char	*e1p, *e2p ;
	int		n ;

	Arguments:

	e1p		first string
	e2p		second string
	n		maximum number of characters to compare

	Returns:

	>0		the second key is greater than the first
	0		the keys of the strings are equal
	<0		the first key is less than the second


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */

#define	KEYEND(c)	(((c) == '\0') || ((c) == '='))


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int strnkeycmp(cchar *e1p,cchar *e2p,int n)
{
	int		rc = 0 ;

	if ((e1p == NULL) && (e2p == NULL))
	    return 0 ;

	if (e1p == NULL)
	    return 1 ;

	if (e2p == NULL)
	    return -1 ;

#if	CF_DEBUGS
	debugprintf("strnkeycmp: n=%d s1=%s s2=%t\n",
		n,e1p,e2p,strnlen(e2p,40)) ;
#endif

	if (n >= 0) {

	    while (*e1p && *e2p && (n > 0)) {

	        if ((*e1p == '=') || (*e2p == '='))
	            break ;

		rc = *e1p - *e2p ;
	        if (rc != 0)
	            break ;

	        e1p += 1 ;
	        e2p += 1 ;
	        n -= 1 ;

	    } /* end while */

	} else {

	    while (*e1p && *e2p) {

	        if ((*e1p == '=') || (*e2p == '='))
	            break ;

	        rc = *e1p - *e2p ;
	        if (rc != 0)
	            break ;

	        e1p += 1 ;
	        e2p += 1 ;

	    } /* end while */

	} /* end if */

	if (n != 0) {
	    rc = 0 ;
	    if (*e1p != *e2p) {
	        if ((rc == 0) && (*e1p == '=')) {
	            rc = (*e2p == '\0') ? 0 : -1 ;
	        }
	        if ((rc == 0) && (*e2p == '=')) {
	            rc = (*e1p == '\0') ? 0 : 1 ;
	        }
	        if (rc == 0) rc = (*e1p - *e2p) ;
	    }
	}

	return rc ;
}
/* end subroutine (strnkeycmp) */


