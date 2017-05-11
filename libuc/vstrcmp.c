/* vstrcmp */

/* vector string key comparison */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is suitable for uses such as with 'qsort(3c)' or
        'bsearch(3c)' and 'vecstr_finder(3dam)'.


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


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


/* compare routine for environment variable search */
int vstrcmp(e1p,e2p)
const char	**e1p, **e2p ;
{
	int	rc ;


	if ((*e1p == NULL) && (*e2p == NULL)) 
		return 0 ;

	if (*e1p == NULL) 
		return 1 ;

	if (*e2p == NULL) 
		return -1 ;

	rc = strcmp(*e1p,*e2p) ;

#if	CF_DEBUGS
	debugprintf("vstrcmp: ret rc=%d\n",rc) ;
#endif

	return rc ;
}
/* end subroutine (vstrcmp) */


