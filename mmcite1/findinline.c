/* findinline */

/* find a TeX-type in-line text escape */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was adapted from others programs that did similar types
        of functions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds a TeX-like in-line text escape.

	Synopsis:

	int findinline(fip,lp,ll)
	FINDINLINE	*fip ;
	const char	*lp ;
	int		ll ;

	Arguments:

	fip	pinter to FINDINLINE structure
	lp	supplied string to test
	ll	length of supplied string to test

	Returns:

	>=0	length of result "thing" 
	<0	error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"findinline.h"


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff)
#endif


/* external subroutines */

extern int	isalphalatin(int) ;
extern int	isalnumlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnsub(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	getdash(FINDINLINE *,const char *,int) ;
static int	getpair(FINDINLINE *,const char *,int) ;


/* local variables */


/* exported subroutines */


int findinline(FINDINLINE *fip,const char *lp,int ll)
{
	int		skiplen = 0 ;

	if (fip == NULL) return SR_FAULT ;
	if (lp == NULL) return SR_FAULT ;

	if (ll < 0) ll = strlen(lp) ;

	if (ll >= 3) {
	    const char	*tp, *cp ;
	    int		cl ;
	    while ((tp = strnchr(lp,ll,CH_BSLASH)) != NULL) {
		fip->sp = tp ;
		cp = (tp+1) ;
		cl = ((lp+ll)-(tp+1)) ;

		if (cl > 0) {
		    int	ch = MKCHAR(cp[0]) ;
		    if (ch == '_') {
	        	skiplen = getdash(fip,cp,cl) ;
		    } else if (isalphalatin(ch)) {
	        	skiplen = getpair(fip,cp,cl) ;
		    }
	      	    if (skiplen > 0) break ;
		} /* end if */

	        lp += 1 ;
	        ll -= 1 ;
	    } /* end while */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("findinline: ret sl=%d\n",skiplen) ;
#endif

	return skiplen ;
}
/* end subroutine (findinline) */


/* local subroutines */


static int getdash(FINDINLINE *fip,const char *sp,int sl)
{
	int		skiplen = 0 ;
	const char	*start = (sp-1) ;

	fip->kp = sp ;
	fip->kl = 1 ;

	sp += 1 ;
	sl -= 1 ;
	if (sl > 0) {
	    int	vl = 0 ;
	    fip->vp = sp ;
	    while (sl && isalphalatin(MKCHAR(sp[0]))) {
		sp += 1 ;
		sl -= 1 ;
		vl += 1 ;
	    }
	    if (vl > 0) {
		fip->vl = vl ;
		skiplen = (sp-start) ;
	    }
	} /* end if */

	return skiplen ;
}
/* end subroutine (getdash) */


static int getpair(FINDINLINE *fip,const char *sp,int sl)
{
	int		skiplen = 0 ;
	const char	*start = (sp-1) ;
	const char	*tp ;

	fip->kp = sp ;
	if ((tp = strnchr(sp,sl,CH_LBRACE)) != NULL) {
	    int		kl ;
	    if ((kl = sfshrink(sp,(tp-sp),NULL)) > 0) {
		fip->kl = kl ;
		sl -= ((tp+1)-sp) ;
		sp = (tp+1) ;
		fip->vp = sp ;
	        if ((tp = strnchr(sp,sl,CH_RBRACE)) != NULL) {
		    if ((tp-sp) > 0) {
			fip->vl = (tp-sp) ;
			skiplen = ((tp+1)-start) ;
		    }
		}
	    }
	} /* end if */
		     
	return skiplen ;
}
/* end subroutine (getpair) */


