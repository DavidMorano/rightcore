/* random */

/* RANDOM object */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-23, David A­D­ Morano
        I made this object to mimic the behavior of the UNIX® System RNG
        'random()' but to make it thread safe (and object oriented)! So many
        stupid subroutines in UNIX® are not thread safe and there is no need for
        it all! But those Computer Science types who were so instrumental in
        developing UNIX® and its libraries in the first place were not the
        smartest when it came to production-grade computer software! We now have
        to live with their mistakes!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a knock-off of the UNIX® System 'random(3)' library RNG.  I
	support the same "types" as it did ; namely five in all with type zero
	being the old stupid LC RNG that produces crap!  If you want it, it is
	there.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"random.h"


/* local defines */

#define	TYPE_0		0		/* linear congruential */
#define	DEG_0		0
#define	SEP_0		0

#define	TYPE_1		1		/* x**7 + x**3 + 1 */
#define	DEG_1		7
#define	SEP_1		3

#define	TYPE_2		2		/* x**15 + x**1 + 1 */
#define	DEG_2		15
#define	SEP_2		1

#define	TYPE_3		3		/* x**31 + x**3 + 1 */
#define	DEG_3		31
#define	SEP_3		3

#define	TYPE_4		4		/* x**63 + x**1 + 1 */
#define	DEG_4		63
#define	SEP_4		1

#define	MAX_TYPES	5		/* max number of types above */


/* external subroutines */

extern int	randlc(int) ;


/* local variables */

static uint randtbl[] = {
	0x991539b1, 0x16a5bce3, 0x6774a4cd, 0x3e01511e, 
	0x4e508aaa, 0x61048c05, 0xf5500617, 0x846b7115, 
	0x6a19892c, 0x896a97af, 0xdb48f936, 0x14898454,
	0x37ffd106, 0xb58bff9c, 0x59e17104, 0xcf918a49, 
	0x09378c83, 0x52c7a471, 0x8d293ea9, 0x1f4fc301, 
	0xc3db71be, 0x39b44e1c, 0xf8a44ef9, 0x4c8b80b1,
	0x19edc328, 0x87bf4bdd, 0xc9b240e5, 0xe9ee4b1b, 
	0x4382aee7, 0x535b6b41, 0xf3bec5da, 0
} ;

static int degrees[MAX_TYPES] =	{
	DEG_0, DEG_1, DEG_2, DEG_3, DEG_4 
} ;

static int seps[MAX_TYPES] =	{
	SEP_0, SEP_1, SEP_2, SEP_3, SEP_4 
} ;


/* exported subroutines */


int random_start(RANDOM *rp,int type,uint seed)
{
	ULONG		hi, lo ;
	uint		ihi, ilo, uiw ;
	int		i ;

	if (rp == NULL)
	    return SR_FAULT ;

	memset(rp,0,sizeof(RANDOM)) ;

	if (type >= MAX_TYPES)
	    return SR_NOTSUP ;

	if (type < 0)
	    type = TYPE_4 ;

	rp->rand_type = type ;
	rp->rand_deg = degrees[type] ;

	rp->rand_sep = seps[type] ;

	rp->fptr = rp->state + rp->rand_sep ;
	rp->rptr = rp->state ;
	rp->end_ptr = rp->state + rp->rand_deg ;

	if (seed == 0) {

	    for (i = 0 ; randtbl[i] != 0 ; i += 1) {

	        uiw = (uint) randlc(randtbl[i]) ;

	        hi = uiw ;
	        lo = randtbl[i] ;

	        rp->state[i] = (hi << 32) | lo ;

	        if (type == TYPE_0) break ;
	    } /* end for */

	} else {

	    for (i = 0 ; i < 64 ; i += 1) {

	        ihi = seed ;
	        seed = (uint) randlc(seed) ;

	        ilo = seed ;
	        seed = (uint) randlc(seed) ;

	        hi = ihi ;
	        lo = ilo ;

	        rp->state[i] = (hi << 32) | lo ;

	        if (type == TYPE_0) break ;
	    } /* end for */

	} /* end if (seeding) */

	return SR_OK ;
}
/* end subroutine (random_start) */


int random_finish(RANDOM *rp)
{

	if (rp == NULL)
	    return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (random_finish) */


int random_getuint(RANDOM *rp,uint *uip)
{
	uint		rv ;

	if (rp->rand_type == TYPE_0) {

	    rv = rp->state[0] ;
	    rv = (uint) randlc(rv) ;

	    rp->state[0] = rv ;

	} else {

	    *rp->fptr += *rp->rptr ;
	    rv = (uint) *rp->fptr ;

	    if (++rp->fptr >= rp->end_ptr) {
	        rp->fptr = rp->state ;
	        rp->rptr += 1 ;
	    } else if (++rp->rptr >= rp->end_ptr) {
	        rp->rptr = rp->state ;
	    }

	} /* end if */

	*uip = rv ;
	return SR_OK ;
}
/* end subroutine (random_getuint) */


int random_getint(RANDOM *rp,int *ip)
{
	int		rv ;

	if (rp->rand_type == TYPE_0) {

	    rv = (int) rp->state[0] ;
	    rv = (int) randlc(rv) ;

	    rp->state[0] = (ULONG) rv ;

	} else {

	    *rp->fptr += *rp->rptr ;
	    rv = (int) (*rp->fptr >> 1) & INT_MAX ;

	    if (++rp->fptr >= rp->end_ptr) {
	        rp->fptr = rp->state ;
	        rp->rptr += 1 ;
	    } else if (++rp->rptr >= rp->end_ptr) {
	        rp->rptr = rp->state ;
	    }

	} /* end if */

	*ip = (int) rv ;
	return SR_OK ;
}
/* end subroutine (random_getint) */


int random_getulong(RANDOM *rp,ULONG *ulp)
{
	ULONG		rv, hi, lo ;
	uint		ihi, ilo ;

	if (rp->rand_type == TYPE_0) {

	    ihi = (uint) (rp->state[0]) ;
	    ilo = (uint) (rp->state[0] >> 32) ;

	    ihi = (uint) randlc(ihi) ;

	    ilo = (uint) randlc(ilo) ;

	    hi = ihi ;
	    lo = ilo ;
	    rv = (hi << 32) | lo ;
	    rp->state[0] = rv ;

	} else {

#if	CF_DEBUGS
	    debugprintf("random: f=%08x r=%08x\n",*fptr,*rptr) ;
#endif

	    *rp->fptr += *rp->rptr ;
	    rv = *rp->fptr ;

	    if (++rp->fptr >= rp->end_ptr) {
	        rp->fptr = rp->state ;
	        rp->rptr += 1 ;
	    } else if (++rp->rptr >= rp->end_ptr) {
	        rp->rptr = rp->state ;
	    }

	} /* end if */

	*ulp = rv ;
	return SR_OK ;
}
/* end subroutine (random_getulong) */


