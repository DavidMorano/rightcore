/* alevel */

/* audio level object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	F_VSTRSORT	0		/* heap sort instead of quick sort ? */
#define	F_SWAPREDUCE	1


/* revision history:

	- 95/12/01, David A­D­ Morano

	Module was originally written.


*/


/**************************************************************************

	This module processes sound samples (provided in floating point
	single-precision format) and records the levels that are found.


**************************************************************************/


#define	ALEVEL_MASTER	1


#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"alevel.h"



/* local defines */



/* external subroutines */


/* forward referecens */






int alevel_init(op)
ALEVEL		*op ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	memset(op,0,sizeof(ALEVEL)) ;

ret0:
	return rs ;
}
/* end subroutine (alevel_init) */


/* free up the entire vector string data structure object */
int alevel_free(op)
ALEVEL		*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (alevel_free) */


int alevel_zero(op)
ALEVEL		*op ;
{
	int	rs = SR_OK ;
	int	size ;


	if (op == NULL)
	    return SR_FAULT ;

	size = 100 * sizeof(ULONG) ;
	memset(op->hits,0,size) ;

ret0:
	return rs ;
}
/* end subroutine (alevel_zero) */


/* add a string to the vector container */
int alevel_proc(op,fa,n)
ALEVEL		*op ;
const float	fa[] ;
int		n ;
{
	int	i, fi ;


	if (op == NULL)
	    return SR_FAULT ;

	if (fa == NULL)
	    return SR_FAULT ;

	if (n <= 0)
	    return SR_OK ;

	for (i = 0 ; i < n ; i += 1) {

	    if ((fa[i] >= 0.0) && (fa[i] < 1.0)) {

	        fi = (int) (fa[i] * 100.0) ;
	        op->hits[fi] += 1 ;

	    } else
	        op->hits[99] += 1 ;

	} /* end for */

	op->c += n ;

	return SR_OK ;
}
/* end subroutine (alevel_proc) */


/* get a string by its index */
int alevel_getlevel(op,ha)
ALEVEL		*op ;
ULONG		ha[] ;
{
	int	size ;


	if (op == NULL)
	    return SR_FAULT ;

	if (ha == NULL)
	    return SR_FAULT ;

	size = 100 * sizeof(ULONG) ;
	memcpy(ha,op->hits,size) ;

	return SR_OK ;
}
/* end subroutine (alevel_getlevel) */



/* INTERNAL SUBROUTINES */



