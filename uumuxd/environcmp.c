/* environcmp */

/* compare routine for environment variable search */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1


/* revision history:

	= 1994-09-03, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine makes a comparison of the key of a string that 
	looks like a SHELL variable assignment.


*****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	getfname(char *,char *,int,char *) ;
extern int	bopenroot(bfile *,char *,char *,char *,char *,int) ;


/* externals variables */


/* forward references */


/* local global variabes */


/* exported subroutines */


int environcmp(e1p,e2p)
char	*e1p, *e2p ;
{


	if ((e1p == NULL) && (e2p == NULL)) return 0 ;

	if (e1p == NULL) return 1 ;

	if (e2p == NULL) return -1 ;

	while (*e1p && *e2p) {

	    if ((*e1p == '=') || (*e2p == '=')) break ;

	    if (*e1p != *e2p) break ;

		e1p += 1 ;
		e2p += 1 ;
	}

	if (*e1p == *e2p)
	    return 0 ;

	if ((*e1p == '\0') && (*e2p == '='))
	    return 0 ;

	if ((*e2p == '\0') && (*e1p == '='))
	    return 0 ;

	if (*e1p == '\0')
		return -1 ;

	if (*e2p == '\0')
		return 1 ;

	return ((*e1p == '=') ? -1 : 1) ;
}
/* end subroutine (environcmp) */



