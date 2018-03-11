/* cheaprand */

/* cheap random number generator */


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

        This is a cheapy random number generator. There is no theory or proofs
        that this is a "good" RNG. Although this RNG is part of a
        cryptographically strong messages system, this RNG in itself is never
        used as the main component.

        The 'randmac()' subroutine below is a 32 bit RNG that is based loosely
        on the well-performing Multiphy With Carry type generators. However, and
        note carefully, the 'randmac()' subroutine is NOT of that type. A MWC
        adds in the carry from the PREVIOUS multiply where the subroutine below
        adds in the carry from the CURRENT multiply. It appears to perform well
        in RNG tests.

        The 'cheaprand()' subroutine is a combination of two of the 'randmac()'
        RNGs. Again, there is no theory or proof that this is a good RNG either.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* forward references */

static uint	randmac(uint) ;


/* exported subroutines */


ULONG cheaprand(ULONG ow)
{
	ULONG		result ;
	ULONG		lo = (ow >> 00) ;
	ULONG		hi = (ow >> 32) ;
	uint		ihi, ilo ;
	uint		ehi, elo ;

	ihi = (uint) hi ;
	ilo = (uint) lo ;

#if	CF_DEBUGS
	debugprintf("newcheap: 1 hi=%08x lo=%08x\n",ihi,ilo) ;
#endif

	ehi = randmac(ihi) + ilo ;

	elo = randmac(ilo) + ihi ;

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
/* end subroutine (cheaprand) */


/* local subroutines */


static uint randmac(uint v)
{
	ULONG		x = (ULONG) v ;
	ULONG		sum, prod ;
	ULONG		a = 1967773755 ;
	uint		nv ;

	prod = a * x ;
	sum = prod + (prod >> 32) ;

	nv = (uint) sum ;
	return nv ;
}
/* end subroutine (randmac) */


