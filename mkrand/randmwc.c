/* randmwc */

/* randmwc random number generator */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-23, David A­D­ Morano
        I made this subroutine to make a random number (scramble up one really)
        for 64-bit integers. I am not aware of any existing random number
        generator (scrambler) for 64-bit integers so far. I suppose that once
        all machines are 64 bits (and they adhere to the LP64 model) all 'long's
        will be 64 bits without us even knowing about it! Whatever.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object calculates random numbers using a multiply-with-carry
	algorithm.  This algorithm became popular just before
	subtract-with-borrow started to gain popularity.  Obviously subtracting
	with a borrow is cheaper than a multiply with a carry, but hey, this is
	still is use.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"randmwc.h"


/* local defines */

#define	RANDMWC_NA	10


/* external subroutines */

extern int	randlc(int) ;


/* forward references */

static uint	single(uint,uint,uint,uint *) ;


/* local variables */

static const uint	aes[] = {
	1967773755,
	1517746329,
	1447497129,
	1655692410,
	1606218150,
	2051013963,
	1075433238,
	1557985959,
	1781943330,
	1893513180
} ;


/* exported subroutines */


int randmwc_start(RANDMWC *cp,int ai,uint seed)
{
	ULONG		xhi, xlo ;
	ULONG		chi, clo ;
	uint		oxhi, oxlo ;
	uint		ochi, oclo ;
	uint		nxhi, nxlo ;
	uint		nchi, nclo ;

	if (cp == NULL)
	    return SR_FAULT ;

	if (ai < 0) {
	    ai = 0 ;
	} else if (ai >= RANDMWC_NA)
	    return SR_NOTSUP ;

	cp->a = aes[ai] ;
	if (seed == 0)
	    seed = 31415926 ;

	oxlo = (uint) randlc(seed) ;

	oclo = (uint) randlc(oxlo) ;

#if	CF_DEBUGS
	debugprintf("randmwc_start: oxlo=%08x oclo=%08x\n",oxlo,oclo) ;
#endif

	nxlo = single(cp->a,oxlo,oclo,&nclo) ;

	oxhi = (uint) randlc(nxlo) ;

	ochi = (uint) randlc(oxhi) ;

#if	CF_DEBUGS
	debugprintf("randmwc_start: oxhi=%08x ochi=%08x\n",oxhi,ochi) ;
#endif

	nxhi = single(cp->a,oxhi,ochi,&nchi) ;

	xlo = (ULONG) nxlo ;
	xhi = (ULONG) nxhi ;

	clo = (ULONG) nclo ;
	chi = (ULONG) nchi ;

	cp->x = (xhi << 32) | xlo ;
	cp->c = (chi << 32) | clo ;

#if	CF_DEBUGS
	debugprintf("randmwc_start: x=%016llx o=%016llx\n",cp->x,cp->c) ;
#endif

	return SR_OK ;
}
/* end subroutine (randmwc_start) */


int randmwc_finish(RANDMWC *cp)
{

	if (cp == NULL)
	    return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (randmwc_finish) */


int randmwc_getulong(RANDMWC *cp,ULONG *ulp)
{
	ULONG		xhi, xlo ;
	ULONG		chi, clo ;
	uint		oxhi, oxlo ;
	uint		ochi, oclo ;
	uint		nxhi, nxlo ;
	uint		nchi, nclo ;

	oxhi = cp->x >> 32 ;
	ochi = cp->c >> 32 ;

	oxlo = cp->x ;
	oclo = cp->c ;

	nxhi = single(cp->a,oxhi,ochi,&nchi) ;

	nxlo = single(cp->a,oxlo,oclo,&nclo) ;

	xhi = (ULONG) nxhi ;
	xlo = (ULONG) nxlo ;

	chi = (ULONG) nchi ;
	clo = (ULONG) nclo ;

#if	CF_DEBUGS
	debugprintf("randmwc_getulong: xhi=%016llx xlo=%016llx\n",xhi,xlo) ;
	debugprintf("randmwc_getulong: chi=%016llx clo=%016llx\n",chi,clo) ;
#endif

	cp->x = (xhi << 32) | xlo ;
	cp->c = (chi << 32) | clo ;

	*ulp = cp->x ;
	return SR_OK ;
}
/* end subroutine (randmwc_getulong) */


/* private subroutines */


static uint single(uint a,uint ox,uint oc,uint *ncp)
{
	ULONG		xx = (ULONG) ox ;
	ULONG		cc = (ULONG) oc ;
	ULONG		ss ;
	ULONG		aa = a ;
	uint		nx ;

	ss = ((aa * xx) + cc) ;

	*ncp = (ss >> 32) ;
	nx = (uint) ss ;

	return nx ;
}
/* end subroutine (single) */


