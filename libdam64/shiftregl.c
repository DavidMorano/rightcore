/* shiftregl */

/* Shift Register of Longs */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revistion history:

	= 01/07/10, David A­D­ Morano

	This code was originally started (for Levo machine deadlock
	detection).


*/


/******************************************************************************

	This object just implements a vanilla shift register (not
	clocked).  It should be used in data analysis applications and not
	as a Levo machine hardware component.  There is no levo specific
	aspects to this object (like there has to be for regular Levo
	machine hardware components).


******************************************************************************/


#define	SHIFTREGL_MASTER	0


#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>

#include	"shiftregl.h"


/* local defines */

#define	SHIFTREGL_MAGIC		0x74896233


/* external subroutines */


/* exported subroutines */


int shiftregl_start(srp,n)
SHIFTREGL	*srp ;
int		n ;
{
	int	rs ;
	int	i ;
	int	size ;


	if (srp == NULL)
	    return SR_FAULT ;

	if (n < 1)
	    n = 1 ;

	memset(srp,0,sizeof(SHIFTREGL)) ;

	size = n * sizeof(ULONG) ;
	rs = uc_malloc(size,&srp->regs) ;
	if (rs < 0) goto bad ;

	srp->n = n ;
	for (i = 0 ; i < srp->n ; i += 1)
	    srp->regs[i] = i ;

	srp->magic = SHIFTREGL_MAGIC ;
	return SR_OK ;

bad:
	return rs ;
}


int shiftregl_finish(srp)
SHIFTREGL	*srp ;
{


	if (srp == NULL)
	    return SR_FAULT ;

	if (srp->magic != SHIFTREGL_MAGIC)
	    return SR_NOTOPEN ;

	if (srp->regs != NULL)
	    uc_free(srp->regs) ;

	srp->magic = 0 ;
	return SR_OK ;
}


int shiftregl_shiftin(srp,val)
SHIFTREGL	*srp ;
ULONG		val ;
{
	int	i ;


	if (srp == NULL)
	    return SR_FAULT ;

	if (srp->magic != SHIFTREGL_MAGIC) {

#if	CF_DEBUGS
	    debugprintf("shiftregl_shiftin: m=%08x should be %08x\n",
	        srp->magic,SHIFTREGL_MAGIC) ;
#endif

	    return SR_NOTOPEN ;
	}

	srp->regs[srp->i] = val ;
	srp->i = (srp->i + 1) % srp->n ;
	return SR_OK ;
}


int shiftregl_get(srp,i,vp)
SHIFTREGL	*srp ;
int		i ;
ULONG		*vp ;
{


	if (srp == NULL)
	    return SR_FAULT ;

	if (srp->magic != SHIFTREGL_MAGIC)
	    return SR_NOTOPEN ;

	if (vp == NULL)
	    return SR_FAULT ;

	if (i < 0)
	    return SR_INVALID ;

	if (i >= srp->n)
	    return SR_NOTFOUND ;

	*vp = srp->regs[i] ;
	return SR_OK ;
}
/* end subroutine (shiftregl_get) */


