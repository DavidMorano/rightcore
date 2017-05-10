/* cfdect */

/* convert from a decimal string with time codes on the end */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine converts a character string representing a
	number into an integer.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;
extern int	cfdeci(const char *,int,int *) ;

extern char	*strnpbrk(const char *,int,const char *) ;


/* local structures */


/* forward references */

static int	convert(const char *,int,int,int *) ;


/* local variables */


/* exported subroutines */


int cfdecti(buf,buflen,rp)
const char	buf[] ;
int		buflen ;
int		*rp ;
{
	int	rs = 0 ;
	int	sl ;
	int	f_negative = FALSE ;

	char	*tp, *sp ;


	if (buf == NULL)
	    return SR_FAULT ;

	if (rp == NULL)
	    return SR_FAULT ;

	*rp = 0 ;

	sp = (char *) buf ;
	sl = strnlen(buf,buflen) ;

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

	while ((tp = strnpbrk(sp,sl,"YMDdhms")) != NULL) {

	    rs = convert(sp,(tp - sp),(int) *tp,rp) ;
	    if (rs < 0)
	        break ;

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;

	} /* end while */

	if ((rs >= 0) && (sl > 0))
	    rs = convert(sp,sl,0,rp) ;

/* handle the sign if there was one */

	if (f_negative)
	    *rp = (- *rp) ;

	return rs ;
}
/* end subroutine (cfdecti) */


/* local subroutines */


static int convert(sp,sl,mc,rp)
const char	*sp ;
int		sl ;
int		mc ;
int		*rp ;
{
	int	rs = 0 ;
	int	mf ;
	int	v = 0 ;
	int	cl ;

	char	*cp ;


	mf = 1 ;
	cl = sfshrink(sp,sl,&cp) ;

	if (cl == 0)
	    goto ret0 ;

	switch (mc) {

	case 'Y':
	    mf = 365 * 24 * 60 * 60 ;
	    break ;

	case 'M':
	    mf = 31 * 24 * 60 * 60 ;
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

	rs = cfdeci(cp,cl,&v) ;

	if (rs >= 0)
	    *rp += (v * mf) ;

ret0:
	return rs ;
}
/* end subroutine (convert) */



