/* mkstrunique */
/* lang=C++11 */

/* test whether a string consists of all unique characters */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We modify the given sring in place to remove any duplicates found.

	Synopsis:

	int mkstrunique(char *bp,int bl)

	Arguments:

	bp		string to test
	bl		length of string to test

	Returns:

	-		length of given string


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

static void	bool_init(bool *,int) ;


/* local variables */


/* exported subroutines */


int mkstrunique(char *bp,int bl)
{
	if (bl > 1) {
	    bool	seen[256] ;
	    int		ch ;
	    int		tail = 1 ;
	    bool_init(seen,256) ;
	    while (bl-- && *bp) {
	        ch = MKCHAR(*bp) ;
		if (!seen[ch]) {
		    bp[tail++] = ch ;
	            seen[ch] = true ;
		}
	        bp += 1 ;
	    } /* end while */
	    bp[tail] = '\0' ;
	}
	return bl ;
}
/* end subroutine (mkstrunique) */


/* local subroutines */


static void bool_init(bool *bp,int bl)
{
	int	i ;
	for (i = 0 ; i < bl ; i += 1) bp[i] = false ;
}
/* end subroutine (bool_init) */


