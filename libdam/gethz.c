/* gethz */

/* get the machine HZ */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	This is a spin off of various programs that needed to get the machine
	HZ value.

	= 2018-10-03, David A.D. Morano
	I enhanced this to retain a cached value for each type of HZ request
	individually. This should have been the semantic from day-one.

*/

/* Copyright © 2001,2018 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine retrieves the system HZ value (by possibly different
	means, specified by the caller) but caches the retrieved value
	(individually for each method). We use the system subroutine
	|uc_gethz(3uc)| to get values we do not already have.


	Synopsis:

	int gethz(int w)

	Arguments:

	w		which source to use:
				0 -> any
				1 -> 'HZ' define only
				2 -> 'CLK_TCK' define only
				3 -> |sysconf(3c)| 'CLK_TCK' only

	Returns:

	<0		error
	==0		?
	>0		HZ value


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	GETHZ		struct gethz
#define	GETHZ_NUM	4		/* current implimentation */


/* local structures */

struct gethz {
	int		hz[GETHX_NUM] ;
} ;


/* forward references */


/* local variables */

static GETHZ		gethz_data ; /* zero-initialized */


/* exported subroutines */


int gethz(int w)
{
	GETHZ		*op = &gethz_data ;
	int		rs ;
	if (w < GETHZ_NUM) {
	    if (op->hz[w] == 0) {
	        if ((rs = uc_getval(w)) >= 0) {
	            op->hz[w] = rs ;
	        }
	    } else {
	        rs = op->hz[w] ;
	    }
	} else {
	    rs = SR_NOSYS ;
	}
	return rs ;
}
/* end subroutine (gethz) */

