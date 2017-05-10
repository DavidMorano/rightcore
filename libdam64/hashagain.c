/* hashagain */

/* create a hash from an exiting hash */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_RANDLC	1		/* use 'randlc(3dam)' */


/* revision history:

	= 2001-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine takes an old (existing) hash and creates a new one
	based on some other factors passed.

	Synopsis:

	unsigned int hashagain(ohash,c,nskip)
	uint		ohash ;
	int		c ;
	int		nskip ;

	Arguments:

	ohash		hash to use in calculation
	c		re-hash count 
	nskip		number of times to allow skipping around

	Returns:

	value		the hash value (unsigned)


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* local defines */

#define	NROT	-5			/* rotation amount (in bits) */


/* external subroutines */

extern uint	urotate(uint,int) ;

extern int	randlc(int) ;


/* forward references */


/* exported subroutines */


uint hashagain(uint ohash,int c,int nskip)
{
	uint		nhash = ohash ;

	if (c < nskip) {

#if	CF_RANDLC
	    nhash = randlc(ohash + c) ;
#else
	    nhash = urotate(ohash,NROT) + c ;
#endif /* CF_RANDLC */

	    if (ohash == nhash)
		nhash = (ohash + 1) ;

	} else
	    nhash = (ohash + 1) ;

	return nhash ;
}
/* end subroutine (hashagain) */


