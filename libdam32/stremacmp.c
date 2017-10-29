/* stremacmp */

/* string key comparison */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We compare two EMA strings.

	Synopsis:

	int stremacmp(e1p,e2p)
	const char	*e1p, *e2p ;

	Arguments:

	e1p		first string
	e2p		second string

	Returns:

	>0		the second is greater than the first
	0		the strings are equal
	<0		the first is less than the second


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	CH_AT
#define	CH_AT	'@'
#endif


/* external subroutines */

extern int	strnncmp(cchar *,int,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	stremacmp_main(cchar *,cchar *) ;


/* local variables */


/* exported subroutines */


int stremacmp(cchar *e1p,cchar *e2p)
{
	int		rc = 0 ;
	if ((e1p != NULL) || (e2p != NULL)) {
	    if (e1p != NULL) {
		if (e1p != NULL) {
		    rc = stremacmp_main(e1p,e2p) ;
		} else
		    rc = -1 ;
	    } else
		rc = 1 ;
	}
	return rc ;
}
/* end subroutine (stremacmp) */


/* local subroutines */


static int stremacmp_main(cchar *e1p,cchar *e2p)
{
	int		rc = 0 ;
	cchar		*t1p = strchr(e1p,CH_AT) ;
	cchar		*t2p = strchr(e2p,CH_AT) ;
	if ((t1p != NULL) && (t2p != NULL)) {
	    if ((rc = strcasecmp((t1p+1),(t2p+1))) == 0) {
	        rc = strnncmp(e1p,(t1p-e1p),e2p,(t2p-e2p)) ;
	    }
	} else {
	    rc = strcmp(e1p,e2p) ;
	}
	return rc ;
}
/* end subroutine (stremacmp_main) */


