/* cheap */

/* cheap random number generator */


#define	CF_DEBUGS	0


/* revision history:

	= 1993-04-23, David A­D­ Morano

	I made this subroutine to make a random number (scramble up one
	really) for 64-bit integers.  I am not aware of any existing
	random number generator (scrambler) for 64-bit integers so
	far.  I suppose that once all machines are 64 bits (and they
	adhere to the LP64 model) all 'long's will be 64 bits without
	us even knowing about it !!  Whatever.


*/

/******************************************************************************

	This object implements a "cheap" random number generator.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"cheap.h"


/* forward references */

ULONG		newcheap(ULONG) ;

static uint	randlcu(uint) ;

static int	randlc(int) ;


/* exported subroutines */


int cheap_start(cp,seed)
CHEAP	*cp ;
uint	seed ;
{
	ULONG	lo, hi ;


	if (seed == 0)
		seed = 31415926 ;

	lo = (ULONG) seed ;
	hi = (ULONG) randlcu(seed) ;

	cp->ow = (hi << 32) | lo ;

#if	CF_DEBUGS
	debugprintf("cheap_start: ow=%016llx\n",
		cp->ow) ;
#endif

	return SR_OK ;
}
/* end subroutine (cheap_start) */


int cheap_finish(cp)
CHEAP	*cp ;
{


	return SR_OK ;
}
/* end subroutine (cheap_finish) */


int cheap_getulong(cp,ulp)
CHEAP	*cp ;
ULONG	*ulp ;
{


#if	CF_DEBUGS
	debugprintf("cheap_getulong: ow=%016llx\n",
		cp->ow) ;
#endif

	*ulp = newcheap(cp->ow) ;

#if	CF_DEBUGS
	debugprintf("cheap_getulong: nw=%016llx\n",
		*ulp) ;
#endif

	cp->ow = *ulp ;
	return SR_OK ;
}
/* end subroutine (cheap_getulong) */


/* other API */


ULONG newcheap(ow)
ULONG	ow ;
{
	ULONG	result ;
	ULONG	hi, lo ;

	uint	ihi, ilo ;
	uint	ehi, elo ;


	hi = ow >> 32 ;
	lo = ow ;

	ihi = (uint) hi ;
	ilo = (uint) lo ;

#if	CF_DEBUGS
	debugprintf("newcheap: 1 hi=%08x lo=%08x\n",ihi,ilo) ;
#endif

	ehi = randlcu(ihi) + ilo ;

	elo = randlcu(ilo) + ihi ;

	hi = (ULONG) ehi ;
	lo = (ULONG) elo ;

#if	CF_DEBUGS
	debugprintf("newcheap: ehi=%08x elo=%08x\n",ehi,elo) ;
#endif

#if	CF_DEBUGS
	debugprintf("newcheap: 2a hi=%016llx lo=%016llx\n",hi,lo) ;
	debugprintf("newcheap: 2b hi=%08x lo=%08x\n",(uint) hi,(uint) lo) ;
#endif

	result = (hi << 32) | lo ;
	return result ;
}
/* end subroutine (newcheap) */


uint cheaper(v)
uint	v ;
{
	uint	nv ;


	nv = randlcu(v) ;

	return nv ;
}
/* end subroutine (a cheaper RNG) */



/* LOCAL SUBROUTINES */



static uint randlcu(v)
uint	v ;
{
	ULONG	sum, prod, x ;
	ULONG	a = 1967773755 ;

	uint	nv ;


	if (v == 0)
		v = 31415926 ;

	x = (ULONG) v ;
	prod = a * x ;
	sum = prod + (prod >> 32) ;

	nv = (uint) sum ;
	return nv ;
}
/* end subroutine (randlcu) */


/* standard positive random number (do not touch !) */
static int randlc(v)
int	v ;
{
	register int	hi, lo ;


	if (v <= 0)
		v = 31415926 ^ v ;

	hi = v / 127773 ;
	lo = v % 127773 ;
	v = (16807 * lo) - (2836 * hi) ;
	if (v <= 0)
	    v += INT_MAX ;

	return v ;
}
/* end subroutine (randlc) */



