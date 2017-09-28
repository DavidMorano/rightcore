/* cfroman */
/* lang=C99 */

/* convert from a Roman-Numeral representation to its binary integer value */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2017-09-20, David A­D­ Morano
	For fun, really. Do not have a real use.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Subroutines to convert strings of Roman-Numerals to binary integers.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpyshrink(char *,int,cchar *,int) ;
extern int	snwcpyopaque(char *,int,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	haswhite(cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

int		cfromani(cchar *,int,int *) ;

static int	conv(cchar *,int,ulonglong *) ;
static int	toval(int) ;


/* local variables */


/* exported subroutines */


int cfroman(cchar *sp,int sl,int *rp)
{
	return cfromani(sp,sl,rp) ;
}
/* end subroutine (cfroman) */


int cfromani(cchar *sp,int sl,int *rp)
{
	ulonglong	val ;
	int		rs ;
	rs = conv(sp,sl,&val) ;
	if (rp != NULL) *rp = (int) val ;
	return rs ;
}
/* end subroutine (cfromani) */


int cfromanl(cchar *sp,int sl,long *rp)
{
	ulonglong	val ;
	int		rs ;
	rs = conv(sp,sl,&val) ;
	if (rp != NULL) *rp = (long) val ;
	return rs ;
}
/* end subroutine (cfromanl) */


int cfromanll(cchar *sp,int sl,longlong *rp)
{
	ulonglong	val ;
	int		rs ;
	rs = conv(sp,sl,&val) ;
	if (rp != NULL) *rp = (longlong) val ;
	return rs ;
}
/* end subroutine (cfromanll) */


/* unsigned */
int cfromanui(cchar *sp,int sl,uint *rp)
{
	ulonglong	val ;
	int		rs ;
	rs = conv(sp,sl,&val) ;
	if (rp != NULL) *rp = (uint) val ;
	return rs ;
}
/* end subroutine (cfromanui) */


/* unsigned */
int cfromanul(cchar *sp,int sl,ulong *rp)
{
	ulonglong	val ;
	int		rs ;
	rs = conv(sp,sl,&val) ;
	if (rp != NULL) *rp = (ulong) val ;
	return rs ;
}
/* end subroutine (cfromanul) */


/* unsigned */
int cfromanull(cchar *sp,int sl,ulonglong *rp)
{
	ulonglong	val ;
	int		rs ;
	rs = conv(sp,sl,&val) ;
	if (rp != NULL) *rp = (ulonglong) val ;
	return rs ;
}
/* end subroutine (cfromanull) */


/* local subroutines */


static int conv(cchar *sbuf,int slen,ulonglong *rp)
{
	int		val = 0 ;
	int		sl ;
	cchar		*sp ;
	if ((sl = sfshrink(sbuf,slen,&sp)) > 0) {
	    int		i = 0 ;
	    int		cval = toval(sp[i]) ;
	    int		nval ;
	    for (i = 0 ; i < sl ; i += 1) {
	        nval = (i < (sl-1)) ? toval(sp[i+1]) : 0 ;
	        if (cval < nval) {
		    val -= cval ;
	        } else {
		    val += cval ;
	        }
		cval = nval ;
	    } /* end for */
	    *rp = val ;
	} /* end if (positive) */
	return 0 ;
}
/* end subroutine (conv) */


static int toval(int ch)
{
	int		val = 0 ;
	switch (ch) {
	case 'I': val = 1 ; break ;
	case 'V': val = 5 ; break ;
	case 'X': val = 10 ; break ;
	case 'L': val = 50 ; break ;
	case 'C': val = 100 ; break ;
	case 'D': val = 500 ; break ;
	case 'M': val = 1000 ; break ;
	} /* end switch */
	return val ;
}
/* end subroutine (toval) */


