/* mkuuid */

/* make UUID (also a specialized object) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for calendar operations.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We create a UUID.

	Synpsis:

	int mkuuid(MKUUID *up,int ver)

	Arguments:

	up		pointer to MKUUID object
	ver		version desired

	Returns:

	<0		error
	>=0		OK



*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<fcntl.h>
#include	<string.h>
#include	<vsystem.h>
#include	<mkuuid.h>
#include	<localmisc.h>


/* local defines */

#define	NWORDS	4


/* external subroutines */

extern int	getrand(void *,int) ;


/* forward references */

static int mkuutime(MKUUID *,uint *) ;
static int mkuuclk(MKUUID *,uint *) ;
static int mkuunode(MKUUID *,uint *) ;


/* exported subroutines */


/* ARGSUSED */
int mkuuid(MKUUID *up,int ver)
{
	const int	rsize = (NWORDS*sizeof(uint)) ;
	uint		rwords[NWORDS+1] ;
	int		rs ;

	memset(up,0,sizeof(MKUUID)) ;

	if ((rs = getrand(rwords,rsize)) >= 0) {
	    up->version = 4 ;
	    mkuutime(up,rwords) ;
	    mkuuclk(up,rwords) ;
	    mkuunode(up,rwords) ;
	} /* end if (reading random) */

	return rs ;
}
/* end subroutine (mkuuid) */


/* local subroutines */


static int mkuutime(MKUUID *up,uint *rwords)
{
	uint64_t	tv = 0 ;
	uint64_t	v ;

	v = (rwords[0] & UINT_MAX) ;
	tv |= v ;

	v = (rwords[1] & UINT_MAX) ;
	tv |= (v << 32) ;

	up->time = (uint64_t) tv ;
	return 0 ;
}
/* end subroutine (mkuutime) */


static int mkuuclk(MKUUID *up,uint *rwords)
{
	uint64_t	v ;

	v = (rwords[2] & UINT_MAX) ;
	v >>= 16 ;

	up->clk = (uint16_t) v ;
	return 0 ;
}
/* end subroutine (mkuuclk) */


static int mkuunode(MKUUID *up,uint *rwords)
{
	uint64_t	nv = 0 ;
	uint64_t	v ;

	v = (rwords[2] & UINT_MAX) ;
	v &= USHORT_MAX ;
	nv |= (v << 32) ;

	v = (rwords[3] & UINT_MAX) ;
	nv |= v ;

	up->node = (uint64_t) nv ;
	return 0 ;
}
/* end subroutine (mkuunode) */


