/* gethz */

/* get the machine HZ */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	This is a spin off of various programs that needed to get the machine
	HZ value.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

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


/* local structures */

struct gethz {
	int		hz ;
} ;


/* forward references */

static int getval(int) ;


/* local variables */

static GETHZ		gethz_data ; /* zero-initialized */


/* exported subroutines */


int gethz(int w)
{
	GETHZ		*op = &gethz_data ;
	int		rs ;
	if ((op->hz == 0) || (w > 0)) {
	    if ((rs = getval(w)) >= 0) {
	        op->hz = rs ;
	    }
	} else {
	    rs = op->hz ;
	}
	return rs ;
}
/* end subroutine (gethz) */


/* local subroutines */


static int getval(int w)
{
	int		rs = SR_INVALID ;
	long		hz = -1 ;

#if	defined(HZ)
	if ((hz < 0) && ((w == 0) || (w == 1))) {
	    rs = SR_OK ;
	    hz = HZ ;
	}
#endif
#if	defined(CLK_TCK) 
	if ((hz < 0) && ((w == 0) || (w == 2))) {
	    rs = SR_OK ;
	    hz = CLK_TCK ;
	}
#endif
#if	defined(_SC_CLK_TCK) 
	if ((hz < 0) && ((w == 0) || (w == 3))) {
	    if ((rs = uc_sysconf(_SC_CLK_TCK,NULL)) >= 0) {
		hz = rs ;
	    }
	}
#endif

	return (rs >= 0) ? hz : rs ;
}
/* end subroutine (getval) */


