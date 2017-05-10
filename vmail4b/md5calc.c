/* md5calc */


/* revision history:

	= 2009-01-20, David A­D­ Morano
	This is new code.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to form a digest of some data using the MD5 hash
        algorithm.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<md5.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int md5calc(ULONG *rp,cchar *sp,int sl)
{
	ULONG		hv = 0 ;
	ULONG		v ;
	int		i ;
	uchar		out[16] ;

	if (rp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	md5_calc(out,(uchar *) sp,(uint) sl) ;

	for (i = 0 ; i < 8 ; i += 1) {
	    v = out[i] ;
	    if (ENDIAN) {
	        hv = ((hv << 8) | v) ;
	    } else {
	        v = (v << (i * 8)) ;
	        hv |= v ;
	    }
	} /* end for */

	*rp = hv ;
	return SR_OK ;
}
/* end subroutine (md5calc) */


