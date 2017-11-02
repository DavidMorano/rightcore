/* hasdupplicate */
/* lang=C++11 */

/* does the given array of elements have duplicate entries */


/* revision history:

	= 2011-10-08, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks if the given array of elements (type 'T') has
        duplicate entries.

	Synopsis:

	int hasdupplicate(const int *sp,int sl)

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
#include	<unordered_set>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* forwards references */


/* exported subroutines */


template<typename T>
bool hasduplicate(const T *sp,int sl)
{
	typename std::unordered_set<T>		visited ;
	int		rs = SR_OK ;
	int		f = false ;
	if (sl > 1) {
	    typename std::unordered_set<T>::iterator	end = visited.end() ;
	    for (int i = 0 ; i < sl ; i += 1) {
	        if (visited.find(*sp) != end) {
		    f = true ;
		    break ;
		} else {
		    visited.insert(*sp) ;
		}
	    } /* end for */
	} /* end if (needed more work) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (hasduplicate) */


