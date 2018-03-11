/* hasdupai (Has-Duplicate-Array-Integers) */
/* lang=C99 */

/* does the given array of integers have duplicate entries */


/* revision history:

	= 1998-10-10, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks if the given array of integers has duplicate
        entries.

	Synopsis:
	int hasdupai(const int *sp,int sl)

	Arguments:
	sp		source array of integers
	sl		length of source array

	Returns:
	<0		error
	==0		no duplicates
	==1		found a duplicate entry


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* forwards references */

static int	vcmp(const void *,const void *) ;


/* exported subroutines */


int hasdupai(const int *sp,int sl)
{
	const int	esize = sizeof(int) ;
	const int	size = ((sl+1)*sizeof(int)) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;
	if (sl > 1) {
	    int		*aa ;
	    if ((rs = uc_malloc(size,&aa)) >= 0) {
	        int	i ;
	        memcpy(aa,sp,size) ;
	        qsort(aa,sl,esize,vcmp) ;
	        for (i = 1 ; i < sl ; i += 1) {
		    f = (aa[i] == aa[i-1]) ;
		    if (f) break ;
		} /* end for */
	        rs1 = uc_free(aa) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a) */
	} /* end if (needed more work) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (hasdupai) */


/* local subroutines */


int vcmp(const void *v1p,const void *v2p)
{
	const int	*i1p = (const int *) v1p ;
	const int	*i2p = (const int *) v2p ;
	return (*i1p - *i2p) ;
}
/* end subroutine (vcmp) */


