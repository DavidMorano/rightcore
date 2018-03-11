/* ba (BitArray) */

/* perform some bit-array type operations */


#define	CF_DEBUGS	0


/* revistion history:

	= 1998-05-27, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module does some bit-array type stuff.

	Notes:

	a) we dynamically create a look-up table using | banum_prepare()|


*******************************************************************************/


#define	BA_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ba.h"


/* local defines */

#define	BA_MAX16	(1 << 16)
#define	BA_BITSPERWORD	(8 * sizeof(ULONG))


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

static int	numbits(int) ;


/* local variables */


/* exported subroutines */


int ba_start(BA *cp,BA_NUM *cnp,int n)
{
	const int	nw = ((n / BA_BITSPERWORD) + 1) ;
	int		rs ;
	int		size ;
	void		*p ;

	if (cp == NULL) return SR_FAULT ;

	cp->cnp = NULL ;

	size = nw * sizeof(ULONG) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    int		i ;
	    cp->a = p ;
	    for (i = 0 ; i < nw ; i += 1) cp->a[i] = 0 ;
	    cp->cnp = cnp ;
	    cp->nbits = n ;
	    cp->nwords = nw ;
	} /* end if */

	return rs ;
}
/* end subroutine (ba_start) */


int ba_setones(BA *cp)
{
	int		size ;

	size = cp->nwords * sizeof(ULONG) ;
	memset(cp->a,(~0),size) ;

	return SR_OK ;
}
/* end subroutine (ba_setones) */


int ba_zero(BA *cp)
{
	int		size ;

#if	CF_DEBUGS
	debugprintf("ba_zero: ent\n") ;
	debugprintf("ba_zero: nwords=%u nbits=%u\n",cp->nwords,cp->nbits) ;
#endif

	size = cp->nwords * sizeof(ULONG) ;
	memset(cp->a,0,size) ;

	return SR_OK ;
}
/* end subroutine (ba_zero) */


int ba_countdown(BA *cp)
{
	int		r = 0 ;
	int		f_borrow ;
	int		f_msb1, f_msb2 ;

	do {
	    f_msb1 = cp->a[r] & INT_MAX ;
	    cp->a[r] -= 1 ;
	    f_msb2 = cp->a[r] & INT_MAX ;
	    f_borrow = (! f_msb1) && f_msb2 ;
	    r += 1 ;
	} while (f_borrow && (r < cp->nwords)) ;

	return SR_OK ;
}
/* end subroutine (ba_countdown) */


int ba_and(BA *cp1,BA *cp2)
{
	int		i, nw ;

	nw = MIN(cp1->nwords,cp2->nwords) ;

	for (i = 0 ; i < nw ; i += 1) {
	    cp1->a[i] = cp1->a[i] & cp2->a[i] ;
	}

	return SR_OK ;
}
/* end subroutine (ba_and) */


int ba_numones(BA *cp)
{
	int		i ;
	int		sum = 0 ;
	int		*na = (cp->cnp)->num ;

	for (i = 0 ; i < cp->nwords ; i += 1) {
	    ULONG	v = cp->a[i] ;
	    sum += na[v & (BA_MAX16 - 1)] ; v >>= 16 ;
	    sum += na[v & (BA_MAX16 - 1)] ; v >>= 16 ;
	    sum += na[v & (BA_MAX16 - 1)] ; v >>= 16 ;
	    sum += na[v & (BA_MAX16 - 1)] ; v >>= 16 ;
	} /* end for */

	return sum ;
}
/* end subroutine (ba_numones) */


int ba_finish(BA *cp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (cp == NULL) return SR_FAULT ;

	if (cp->a != NULL) {
	    rs1 = uc_free(cp->a) ;
	    if (rs >= 0) rs = rs1 ;
	    cp->a = NULL ;
	}

	cp->cnp = NULL ;
	cp->nbits = 0 ;
	cp->nwords = 0 ;
	return rs ;
}
/* end subroutine (ba_finish) */


/* other interfaces */


int banum_prepare(BA_NUM *cnp)
{
	const int	size = (BA_MAX16 * sizeof(int)) ;
	int		rs ;

	if ((rs = uc_malloc(size,&cnp->num)) >= 0) {
	    int		i ;
	    for (i = 0 ; i < BA_MAX16 ; i += 1) {
	        cnp->num[i] = numbits(i) ;
	    }
	}

	return rs ;
}
/* end subroutine (banum_prepare) */


int banum_forsake(BA_NUM *cnp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (cnp->num != NULL) {
	    rs1 = uc_free(cnp->num) ;
	    if (rs >= 0) rs = rs1 ;
	    cnp->num = NULL ;
	}

	return rs ;
}
/* end subroutine (banum_forsake) */


/* private subroutines */


static int numbits(int n)
{
	int		sum = 0 ;

	while (n) {
	    if (n & 1) sum += 1 ;
	    n = n >> 1 ;
	}

	return sum ;
}
/* end subroutine (numbits) */


