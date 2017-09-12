/* cfb26 */

/* convert a base-26 digit string to its binary integer value */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-10-01, David A­D­ Morano
	This subroutine was written adapted from assembly.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Subroutines to convert base-26 strings to binary integers.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#define MAXB26DIG_I	7		/* decimal digits in 'int' */
#define MAXB26DIG_UI	7		/* decimal digits in 'uint' */
#define MAXB26DIG_L	7		/* decimal digits in 'long' */
#define MAXB26DIG_UL	7		/* decimal digits in 'ulong' */
#define MAXB26DIG_L64	14		/* decimal digits in 'long64' */
#define MAXB26DIG_UL64	14		/* decimal digits in 'ulong64' */

#undef	CFB26_WEIGHT
#define	CFB26_WEIGHT	26


/* external subroutines */


/* external variables */


/* local structures */

struct info {
	ULONG		result ;	/* result */
	ULONG		imask ;		/* inverse overflow mask */
	int		maxdigs ;	/* maximum digits */
	int		st ;		/* signed type */
} ;


/* forward references */

int 		cfb26i(cchar *,int,int *) ;

static int	icfb26(struct info *,cchar *,int) ;

static int	isbad(int,int) ;


/* local variables */


/* exported subroutines */


int cfb26(cchar *sp,int sl,int *rp)
{

	return cfb26i(sp,sl,rp) ;
}
/* end subroutine (cfb26) */


/* convert from a base-26 string to a signed integer */
int cfb26i(cchar *sp,int sl,int *rp)
{
	struct info	is ;
	int		rs ;

	is.maxdigs = MAXB26DIG_I ;
	is.st = TRUE ;
	is.imask = (~ INT_MAX) ;
	rs = icfb26(&is,sp,sl) ;

	*rp = (int) is.result ;
	return rs ;
}
/* end subroutine (cfb26i) */


/* convert from a decimal number that will yield an unsigned integer */
int cfb26ui(cchar *sp,int sl,uint *rp)
{
	struct info	is ;
	int		rs ;

	is.maxdigs = MAXB26DIG_UI ;
	is.st = FALSE ;
	is.imask = (~ INT_MAX) ;
	rs = icfb26(&is,sp,sl) ;

	*rp = (uint) is.result ;
	return rs ;
}
/* end subroutine (cfb26ui) */


/* convert from a decimal number that will yield a long integer */
int cfb26l(cchar *sp,int sl,long *rp)
{
	struct info	is ;
	int		rs ;

	is.maxdigs = MAXB26DIG_L ;
	is.st = TRUE ;
	is.imask = (~ LONG_MAX) ;
	rs = icfb26(&is,sp,sl) ;

	*rp = (long) is.result ;
	return rs ;
}
/* end subroutine (cfb26l) */


/* convert from a decimal number that will yield an unsigned long integer */
int cfb26ul(cchar *sp,int sl,ulong *rp)
{
	struct info	is ;
	int		rs ;

	is.maxdigs = MAXB26DIG_UL ;
	is.st = FALSE ;
	is.imask = (~ LONG_MAX) ;
	rs = icfb26(&is,sp,sl) ;

	*rp = (ulong) is.result ;
	return rs ;
}
/* end subroutine (cfb26ul) */


/* convert from a decimal number tp a 64 bit LONG integer */
int cfb26ll(cchar *sp,int sl,LONG *rp)
{
	struct info	is ;
	int		rs ;

	is.maxdigs = MAXB26DIG_L64 ;
	is.st = TRUE ;
	is.imask = (~ LONG64_MAX) ;
	rs = icfb26(&is,sp,sl) ;

	*rp = (LONG) is.result ;
	return rs ;
}
/* end subroutine (cfb26ll) */


/* convert from a decimal number to an unsigned long (64 bit) integer */
int cfb26ull(cchar *sp,int sl,ULONG *rp)
{
	struct info	is ;
	int		rs ;

	is.maxdigs = MAXB26DIG_UL64 ;
	is.st = FALSE ;
	is.imask = (~ LONG64_MAX) ;
	rs = icfb26(&is,sp,sl) ;

	*rp = (ULONG) is.result ;
	return rs ;
}
/* end subroutine (cfb26ull) */


/* local subroutines */


static int icfb26(struct info *ip,cchar *sp,int sl)
{
	ULONG		val, weight ;
	int		rs = SR_OK ;
	int		ch ;
	int		n, i ;
	int		cb ;
	int		f_cc ;

	ip->result = 0 ;
	val = 0 ;
	if (sl < 0)
	    sl = strlen(sp) ;

/* remove white space from the rear of the string */

	while ((sl > 0) && CHAR_ISWHITE(sp[sl - 1])) {
	    sl -= 1 ;
	}

	if (sl == 0)
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("icfb26: ent s=%t\n",s,slen) ;
#endif

/* convert possible digits */

	f_cc = CHAR_ISUC(sp[sl-1]) ;
	cb = (f_cc) ? 'A' : 'a' ;

	n = 0 ;
	weight = 1 ;
	for (i = (sl - 1) ; (rs >= 0) && (i >= 0) ; i -= 1) {

	    ch = sp[i] ;
	    if ((ch == '-') || (ch == '+') || CHAR_ISWHITE(ch))
	        break ;

/* bad digits? */

	    if (isbad(f_cc,ch))
	        rs = SR_INVALID ;

	    if (rs >= 0) {

		n += 1 ;
	        if ((n > ip->maxdigs) && (ch != cb))
	            rs = SR_OVERFLOW ;

	        if (rs >= 0) {

	            if (n == ip->maxdigs) {
	                ULONG	adder ;
	                int	f_msb1, f_msb2, f_msbr ;

	                adder = ((ch - cb) * weight) ;
	                f_msb1 = (val & ip->imask) ? 1 : 0 ;
	                f_msb2 = (adder & ip->imask) ? 1 : 0 ;
	                val += adder ;
	                f_msbr = (val & ip->imask) ? 1 : 0 ;

			if (ip->st) {
	                if (f_msb1 || f_msb2 || f_msbr)
	                    rs = SR_OVERFLOW ;
			} else {
	                if ((f_msb1 && f_msb2) ||
	                    ((f_msb1 || f_msb2) && (! f_msbr)))
	                    rs = SR_OVERFLOW ;
			}

	            } else
	                val += ((ch - cb) * weight) ;

	            weight *= CFB26_WEIGHT ;

#if	CF_DEBUGS
		debugprintf("icfb26: i=%u val=%llu\n",i,val) ;
#endif

	        } /* end if */

	    } /* end if */

	} /* end for */

	if (rs >= 0) {

	    while ((i > 0) && CHAR_ISWHITE(sp[i])) {
	        i -= 1 ;
	    }

	    if ((i >= 0) && (sp[i] == '-')) {
	        val = (- val) ;
	    }

	    ip->result = val ;

	} /* end if */

	return rs ;
}
/* end subroutine (icfb26) */


/* 'cc' is the "character case" */
static int isbad(int f_cc,int ch)
{
	int		f ;
	if (f_cc) {
	    f = (ch < 'A') || (ch > 'Z') ;
	} else {
	    f = (ch < 'a') || (ch > 'z') ;
	}
	return f ;
}
/* end subroutine (isbad) */


