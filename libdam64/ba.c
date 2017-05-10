/* ba (BitArray) */

/* perform some bit-array type operations */


#define	CF_DEBUGS	0


/* revistion history:

	= 1998-05-27, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************

	This module does some bit-array type stuff.


*********************************************************************/


#define	BA_MASTER	1


#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
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


int ba_start(cp,cnp,n)
BA		*cp ;
BA_NUM		*cnp ;
int		n ;
{
	int	rs ;
	int	i, nw ;
	int	size ;


	if (cp == NULL)
	    return SR_FAULT ;

	nw = (n / BA_BITSPERWORD) + 1 ;

	cp->cnp = NULL ;

	size = nw * sizeof(ULONG) ;
	if ((rs = uc_malloc(size,&cp->a)) >= 0) {

	    for (i = 0 ; i < nw ; i += 1) cp->a[i] = 0 ;
	    cp->cnp = cnp ;
	    cp->nbits = n ;
	    cp->nwords = nw ;

	} /* end if */

	return rs ;
}
/* end subroutine (ba_start) */


int ba_setones(cp)
BA		*cp ;
{
	int	size ;


	size = cp->nwords * sizeof(ULONG) ;
	memset(cp->a,(~0),size) ;

	return SR_OK ;
}
/* end subroutine (ba_setones) */


int ba_zero(cp)
BA		*cp ;
{
	int	size ;


#if	CF_DEBUGS
	debugprintf("ba_zero: ent\n") ;
#endif

#if	CF_DEBUGS
	debugprintf("ba_zero: nwords=%u nbits=%u\n",cp->nwords,cp->nbits) ;
#endif

	size = cp->nwords * sizeof(ULONG) ;
	memset(cp->a,0,size) ;

	return SR_OK ;
}
/* end subroutine (ba_zero) */


int ba_countdown(cp)
BA		*cp ;
{
	int	r = 0 ;
	int	f_borrow ;
	int	f_msb1, f_msb2 ;


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


int ba_and(cp1,cp2)
BA		*cp1, *cp2 ;
{
	int	i, nw ;


	nw = MIN(cp1->nwords,cp2->nwords) ;

	for (i = 0 ; i < nw ; i += 1)
	    cp1->a[i] = cp1->a[i] & cp2->a[i] ;

	return SR_OK ;
}
/* end subroutine (ba_and) */


int ba_numones(cp)
BA		*cp ;
{
	int	i ;
	int	sum = 0 ;

	int	*na = (cp->cnp)->num ;


	for (i = 0 ; i < cp->nwords ; i += 1) {
	    sum += na[(cp->a[i] >> 00) & (BA_MAX16 - 1)] ;
	    sum += na[(cp->a[i] >> 16) & (BA_MAX16 - 1)] ;
	} /* end for */

	return sum ;
}
/* end subroutine (ba_numones) */


int ba_finish(cp)
BA		*cp ;
{


	if (cp == NULL)
	    return SR_FAULT ;

	if (cp->a != NULL) {
	    uc_free(cp->a) ;
	    cp->a = NULL ;
	}

	cp->cnp = NULL ;
	cp->nbits = 0 ;
	cp->nwords = 0 ;
	return SR_OK ;
}
/* end subroutine (ba_finish) */


/* other interfaces */


int banum_numprepare(cnp)
BA_NUM		*cnp ;
{
	int	rs ;
	int	i ;
	int	size ;


	size = BA_MAX16 * sizeof(int) ;
	rs = uc_malloc(size,&cnp->num) ;
	if (rs < 0) goto ret0 ;

/* make it */

	for (i = 0 ; i < BA_MAX16 ; i += 1)
	    cnp->num[i] = numbits(i) ;

ret0:
	return rs ;
}
/* end subroutine (banum_numprepare) */


int banum_numforsake(cnp)
BA_NUM	*cnp ;
{


	if (cnp->num != NULL) {
	    uc_free(cnp->num) ;
	    cnp->num = NULL ;
	}

	return SR_OK ;
}
/* end subroutine (banum_numforsake) */


/* private subroutines */


static int numbits(n)
int	n ;
{
	int	sum = 0 ;


	while (n) {
	    if (n & 1) sum += 1 ;
	    n = n >> 1 ;
	}

	return sum ;
}
/* end subroutine (numbits) */



