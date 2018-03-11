/* cfdect */

/* convert from a decimal string with time codes on the end */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine converts a character string representing a number into
	an integer.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;

extern char	*strnpbrk(cchar *,int,cchar *) ;


/* local structures */


/* forward references */

static int	convert(cchar *,int,int,int *) ;


/* local variables */


/* exported subroutines */


int cfdecti(cchar *sbuf,int slen,int *rp)
{
	int		rs = SR_OK ;
	int		sl ;
	int		f_negative = FALSE ;
	cchar		*tp, *sp ;

	if (sbuf == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	*rp = 0 ;

	sp = sbuf ;
	sl = strnlen(sbuf,slen) ;

/* get any sign */

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	if (sl && (*sp == '-')) {
	    f_negative = TRUE ;
	    sp += 1 ;
	    sl -= 1 ;
	}

/* convert the rest */

	while ((tp = strnpbrk(sp,sl,"YMWDwdhms")) != NULL) {

	    rs = convert(sp,(tp - sp),(int) *tp,rp) ;
	    if (rs < 0) break ;

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;

	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    rs = convert(sp,sl,0,rp) ;
	}

/* handle the sign if there was one */

	if (f_negative) {
	    *rp = (- *rp) ;
	}

	return rs ;
}
/* end subroutine (cfdecti) */


/* local subroutines */


static int convert(cchar *sp,int sl,int mc,int *rp)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;

	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    int		mf = 1 ;
	    int		v = 0 ;
	    switch (mc) {
	    case 'Y':
	        mf = 365 * 24 * 60 * 60 ;
	        break ;
	    case 'M':
	        mf = 31 * 24 * 60 * 60 ;
	        break ;
	    case 'W':
	    case 'w':
	        mf = 7 * 24 * 60 * 60 ;
	        break ;
	    case 'D':
	    case 'd':
	        mf = 24 * 60 * 60 ;
	        break ;
	    case 'h':
	        mf = 60 * 60 ;
	        break ;
	    case 'm':
	        mf = 60 ;
	        break ;
	    case 's':
	        break ;
	    } /* end switch */
	    if ((rs = cfdeci(cp,cl,&v)) >= 0) {
	        *rp += (v * mf) ;
	    }
	} /* end if (non-zero) */

	return rs ;
}
/* end subroutine (convert) */


