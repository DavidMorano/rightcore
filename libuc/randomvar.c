/* randomvar */

/* random number generation object */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SAFE		1		/* safety */
#define	CF_SAFE2	0		/* safety (level-2) */


/* revision history:

	= 1998-08-11, David A­D­ Morano
	This object module was originally written.

	= 1999-10-08, David A­D­ Morano
        This module was updated to use 64-bit long integers that have been out
        for a while now (on Alpha and Sparc).

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides a random number generator.  This is similar to the
	UNIX System 'random(3)' subroutine except that no global variables are
	used.  This version also provides a means to get truly random numbers
	by periodically mixing in garbage collected from the machine
	environment.

	We implement a standard cyclical shift register with feedback taps to
	create our random numbers (like 'random(3)' does) but we do it with a
	128 degree polynomial.  Our polynomial is currently:

		x**128 + x**67 + x**23 + 1

	If you have a better one (which is likely) or even a good one, let me
	know!!!  Some known good polynomials for lower degrees than what we are
	working with are:

		 x**7 + x**3 + 1
		x**15 + x**1 + 1
		x**31 + x**3 + 1
		x**63 + x**1 + 1


*******************************************************************************/


#define	RANDOMVAR_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/time.h>		/* |gettimeofday(3c)| */
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"randomvar.h"


/* local defines */

#define	MOD(n)		((n) % RANDOMVAR_DEGREE)
#define	COF(n)		MOD(n)

#define	STIRINTERVAL	(5 * 60)
#define	MAINTCOUNT	1000

#ifndef	VARRANDOM
#define	VARRANDOM	"RANDOM"
#endif

/* external subroutines */

extern int	cfdecui(cchar *,int,uint *) ;
extern int	randlc(int) ;


/* forward references */

int		randomvar_getulong(RANDOMVAR *,ULONG *) ;

static int	randomvar_maint(RANDOMVAR *) ;

static void	addnoise(union randomvar_state *,struct timeval *tvp) ;

static int	rdulong(cchar *,int,ULONG *) ;
static int	wrulong(char *,int,ULONG) ;


/* local variables */


/* exported subroutines */


int randomvar_start(RANDOMVAR *rdp,int f_pseudo,uint seed)
{
	ULONG		dummy ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("randomvar_start: pseudo=%d seed=%08X\n",
	    f_pseudo,seed) ;
#endif

	if (rdp == NULL) return SR_FAULT ;

	if (seed == 0)
	    seed = 31415926 ;

#if	CF_DEBUGS
	debugprintf("randomvar_start: modified_seed=%08X\n",
	    seed) ;
#endif

	rdp->f.flipper = FALSE ;
	rdp->f.pseudo = f_pseudo ;

	rdp->laststir = 0 ;
	rdp->maintcount = 0 ;

	if (rdp->f.pseudo) {

	    rdp->state.is[0] = randlc(seed) ;

	    for (i = 1 ; i < (RANDOMVAR_DEGREE * 2) ; i += 1) {
	        rdp->state.is[i] = randlc(rdp->state.is[i - 1]) ;
	    }

	} else {
	    struct timeval	tv ;
	    const pid_t		pid = getpid() ;
	    const uid_t		uid = getuid() ;
	    uint		v1, v2 ;
	    uint		v3 = 0 ;
	    int			j ;
  	    cchar		*cp ;

	    gettimeofday(&tv,NULL) ;

	    v1 = getppid() ;

	    v2 = getpgrp() ;

	    if ((cp = getenv(VARRANDOM)) != NULL) {
		cfdecui(cp,-1,&v3) ;
	    }

	    i = 0 ;
	    rdp->state.is[i++] += randlc(tv.tv_usec) ;

	    rdp->state.is[i++] += randlc((int) pid) ;

	    rdp->state.is[i++] += randlc(v1) ;

	    rdp->state.is[i++] += randlc(v2) ;

	    rdp->state.is[i++] += randlc(v3) ;

	    rdp->state.is[i++] += randlc(tv.tv_sec) ;

	    rdp->state.is[i++] += randlc(uid) ;

	    for (j = 0 ; j < 6 ; j += 1) {
	        seed ^= rdp->state.is[j] ;
	    }

	    rdp->state.is[i++] += randlc(seed) ;

	    for (j = i ; j < (RANDOMVAR_DEGREE * 2) ; j += 1) {
	        rdp->state.is[j] += randlc(rdp->state.is[j - 1]) ;
	    }

	} /* end if (initializing state) */

#if	CF_DEBUGS
	for (i = 0 ; i < RANDOMVAR_DEGREE ; i += 1)
	    debugprintf("randomvar_start: state[%02d]=%016llX\n",i,
	        rdp->state.ls[i]) ;
#endif /* CF_DEBUGS */

/* our polynomial --  x**128 + x**67 + x**23 + 1  */

	rdp->a = COF(67) ;
	rdp->b = COF(23) ;
	rdp->c = COF(0) ;

	rdp->magic = RANDOMVAR_MAGIC ;

/* stir the pot at least one cycle */

	for (i = 0 ; i < (RANDOMVAR_DEGREE * 20) ; i += 1) {
	    randomvar_getulong(rdp,&dummy) ;
	}

	return SR_OK ;
}
/* end subroutine (randomvar_start) */


int randomvar_finish(RANDOMVAR *rdp)
{

	if (rdp == NULL) return SR_FAULT ;

	if (rdp->magic != RANDOMVAR_MAGIC) return SR_NOTOPEN ;

	rdp->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (randomvar_finish) */


int randomvar_stateload(RANDOMVAR *rdp,cchar *state,int sl)
{
	ULONG		ulw ;
	int		r ;
	int		i ;
	cchar		*sp = (cchar *) state ;

	if (rdp == NULL) return SR_FAULT ;

	if (rdp->magic != RANDOMVAR_MAGIC) return SR_NOTOPEN ;

	if (sl < RANDOMVAR_STATELEN) return SR_INVALID ;

	for (i = 0 ; i < RANDOMVAR_DEGREE ; i += 1) {
	    if ((r = rdulong(sp,sl,&ulw)) > 0) {
	        rdp->state.ls[i] = ulw ;
		sp += r ;
		sl -= r ;
	    } else
		break ;
	} /* end for */

	return SR_OK ;
}
/* end subroutine (randomvar_stateload) */


int randomvar_statesave(RANDOMVAR *rdp,char *state,int bl)
{
	ULONG		ulw ;
	int		r ;
	int		i ;
	char		*bp = (char *) state ;

	if (rdp == NULL) return SR_FAULT ;

	if (rdp->magic != RANDOMVAR_MAGIC) return SR_NOTOPEN ;

	if (bl < RANDOMVAR_STATELEN) return SR_INVALID ;

	for (i = 0 ; i < RANDOMVAR_DEGREE ; i += 1) {
	    ulw = rdp->state.ls[i]  ;
	    if ((r = wrulong(bp,bl,ulw)) > 0) {
		bp += r ;
		bl -= r ;
	    } else
		break ;
	} /* end for */

	return SR_OK ;
}
/* end subroutine (randomvar_statesave) */


int randomvar_addnoise(RANDOMVAR *rdp,const void *noise,int sl)
{
	ULONG		ulw ;
	const int	nmax = (sl / sizeof(ULONG)) ;
	int		r ;
	int		i ;
	int		ii ;
	cchar		*sp = (cchar *) noise ;

	if (rdp == NULL) return SR_FAULT ;

	if (rdp->magic != RANDOMVAR_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; i < nmax ; i += 1) {
	    if ((r = rdulong(sp,sl,&ulw)) > 0) {
	        ii = MOD(i) ;
	        rdp->state.ls[ii] ^= ulw ;
		sp += r ;
		sl -= r ;
	    } else
		break ;
	} /* end for */

	return SR_OK ;
}
/* end subroutine (randomvar_addnoise) */


/* set the polynomial to use (second highest degree to next lowest) */
int randomvar_setpoly(RANDOMVAR *rdp,int a,int b)
{

	if (rdp == NULL) return SR_FAULT ;

	if (rdp->magic != RANDOMVAR_MAGIC) return SR_NOTOPEN ;

	if ((a <= 0) || (a >= RANDOMVAR_DEGREE)) return SR_INVALID ;
	if ((b <= 0) || (b >= RANDOMVAR_DEGREE)) return SR_INVALID ;

	rdp->a = COF(a) ;
	rdp->b = COF(b) ;
	return SR_OK ;
}
/* end subroutine (randomvar_setpoly) */


int randomvar_getlong(RANDOMVAR *rdp,LONG *rp)
{

	if (rdp == NULL) return SR_FAULT ;

	if (rdp->magic != RANDOMVAR_MAGIC) return SR_NOTOPEN ;

#if	CF_SAFE2
	rdp->a = MOD(rdp->a) ;
	rdp->b = MOD(rdp->b) ;
	rdp->c = MOD(rdp->c) ;
#endif

	rdp->state.ls[rdp->a] += rdp->state.ls[rdp->b] ;
	rdp->state.ls[rdp->a] += rdp->state.ls[rdp->c] ;

	*rp = (rdp->state.ls[rdp->a] >> 1) & LONG64_MAX ;

	rdp->a = MOD(rdp->a + 1) ;
	rdp->b = MOD(rdp->b + 1) ;
	rdp->c = MOD(rdp->c + 1) ;

	if (++rdp->maintcount >= MAINTCOUNT)
	    randomvar_maint(rdp) ;

	return SR_OK ;
}
/* end subroutine (randomvar_getlong) */


int randomvar_getulong(RANDOMVAR *rdp,ULONG *rp)
{

	if (rdp == NULL) return SR_FAULT ;

	if (rdp->magic != RANDOMVAR_MAGIC) return SR_NOTOPEN ;

#if	CF_SAFE2
	rdp->a = MOD(rdp->a) ;
	rdp->b = MOD(rdp->b) ;
	rdp->c = MOD(rdp->c) ;
#endif

	rdp->state.ls[rdp->a] += rdp->state.ls[rdp->b] ;
	rdp->state.ls[rdp->a] += rdp->state.ls[rdp->c] ;

	*rp = rdp->state.ls[rdp->a] ;

	rdp->a = MOD(rdp->a + 1) ;
	rdp->b = MOD(rdp->b + 1) ;
	rdp->c = MOD(rdp->c + 1) ;

	if (++rdp->maintcount >= MAINTCOUNT) {
	    randomvar_maint(rdp) ;
	}

	return SR_OK ;
}
/* end subroutine (randomvar_getulong) */


int randomvar_getint(RANDOMVAR *rdp,int *rp)
{

	if (rdp == NULL) return SR_FAULT ;

	if (rdp->magic != RANDOMVAR_MAGIC) return SR_NOTOPEN ;

#if	CF_SAFE2
	rdp->a = MOD(rdp->a) ;
	rdp->b = MOD(rdp->b) ;
	rdp->c = MOD(rdp->c) ;
#endif

	rdp->f.flipper = (! rdp->f.flipper) ;
	if (rdp->f.flipper) {

	    rdp->state.ls[rdp->a] += rdp->state.ls[rdp->b] ;
	    rdp->state.ls[rdp->a] += rdp->state.ls[rdp->c] ;

	    *rp = (rdp->state.is[(rdp->a << 1) + 1] >> 1) & INT_MAX ;

	    rdp->a = MOD(rdp->a + 1) ;
	    rdp->b = MOD(rdp->b + 1) ;
	    rdp->c = MOD(rdp->c + 1) ;

	} else {
	    *rp = (rdp->state.is[rdp->a << 1] >> 1) & INT_MAX ;
	}

	if (++rdp->maintcount >= MAINTCOUNT) {
	    randomvar_maint(rdp) ;
	}

	return SR_OK ;
}
/* end subroutine (randomvar_getint) */


int randomvar_getuint(RANDOMVAR *rdp,uint *rp)
{

	if (rdp == NULL) return SR_FAULT ;

	if (rdp->magic != RANDOMVAR_MAGIC) return SR_NOTOPEN ;

#if	CF_SAFE2
	rdp->a = MOD(rdp->a) ;
	rdp->b = MOD(rdp->b) ;
	rdp->c = MOD(rdp->c) ;
#endif

	rdp->f.flipper = (! rdp->f.flipper) ;
	if (rdp->f.flipper) {

	    rdp->state.ls[rdp->a] += rdp->state.ls[rdp->b] ;
	    rdp->state.ls[rdp->a] += rdp->state.ls[rdp->c] ;

	    *rp = rdp->state.is[(rdp->a << 1) + 1] ;

	    rdp->a = MOD(rdp->a + 1) ;
	    rdp->b = MOD(rdp->b + 1) ;
	    rdp->c = MOD(rdp->c + 1) ;

	} else {
	    *rp = rdp->state.is[rdp->a << 1] ;
	}

	if (++rdp->maintcount >= MAINTCOUNT) {
	    randomvar_maint(rdp) ;
	}

	return SR_OK ;
}
/* end subroutine (randomvar_getuint) */


/* private subroutines */


static int randomvar_maint(RANDOMVAR *rdp)
{
	struct timeval	tv ;
	uint		v0, v1 ;
	int		i ;

	rdp->maintcount = 0 ;
	for (i = 0 ; i < (RANDOMVAR_DEGREE * 2) ; i += 2) {

	    v0 = randlc(rdp->state.is[i + 0]) ;

	    v1 = randlc(rdp->state.is[i + 1]) ;

	    rdp->state.is[i + 0] += v1 ;
	    rdp->state.is[i + 1] += v0 ;

	} /* end for */

	if (! rdp->f.pseudo) {
	    gettimeofday(&tv,NULL) ;
	    if ((tv.tv_sec - rdp->laststir) >= RANDOMVAR_STIRTIME) {
	        rdp->laststir = tv.tv_sec ;
	        addnoise(&rdp->state,&tv) ;
	    }
	} /* end if (not-pseudo) */

	return SR_OK ;
}
/* end subroutine (randomvar_maint) */


static void addnoise(union randomvar_state *sp,struct timeval *tvp)
{
	sp->is[0] ^= randlc(tvp->tv_sec ^ sp->is[0]) ;
	sp->is[1] ^= randlc(tvp->tv_usec ^ sp->is[1]) ;
}
/* end subroutine (addnoise) */


static int rdulong(cchar *sp,int sl,ULONG *lp)
{
	int		r = 0 ;
	if (sl > 0) {
	    ULONG	ulw = 0 ;
	    int		mlen = MIN(sl,sizeof(ULONG)) ;
	    int		i ;
	    for (i = 0 ; i < mlen ; i += 1) {
		ulw <<= 8 ;
		ulw |= ((uchar) *sp++) ;
		r += 1 ;
	    }
	    *lp = ulw ;
	} else {
	    *lp = 0 ;
	}
	return r ;
}
/* end subroutine (rdulong) */


static int wrulong(char *bp,int bl,ULONG ulw)
{
	const int	n = sizeof(ULONG) ;
	int		i ;
	for (i = 0 ; (i < n) && (i < bl) ; i += 1) {
	    *bp = (char) ulw ; ulw >>= 8 ;
	} /* end for */
	return i ;
}
/* end subroutine (wrulong) */


