/* ctwords */
/* lang=C++11 */

/* conver to Number Words */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2017-08-15, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine takes numbers (positive integers) as arguments and
        creates a string containing the necessary words to describe each given
        number.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<cstdlib>
#include	<cinttypes>
#include	<cstring>
#include	<new>
#include	<algorithm>
#include	<string>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* name-spaces */

using namespace std ;


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif


/* global variables */


/* local structures (and methods) */


/* forward references */

static int speak_billions(string *,int) ;
static int speak_millions(string *,int) ;
static int speak_thousands(string *,int) ;
static int speak_group(string *,int) ;
static int speak_hundreds(string *,int) ;
static int speak_tens(string *,int) ;
static int speak_teens(string *,int) ;
static int speak_ones(string *,int) ;


/* local variables */

static cchar	*ones[] = {
	"Zero",
	"One",
	"Two",
	"Three",
	"Four",
	"Five",
	"Six",
	"Seven",
	"Eight",
	"Nine",
	NULL
} ;

static cchar	*teens[] = {
	"Ten",
	"Eleven",
	"Twelve",
	"Thirteen",
	"Fourteen",
	"Fifteen",
	"Sixteen",
	"Seventeen",
	"Eighteen",
	"Nineteen",
	NULL
} ;

static cchar	*tens[] = {
	"and",
	"Ten",
	"Twenty",
	"Thirty",
	"Forty",
	"Fifty",
	"Sixty",
	"Seventy",
	"Eighty",
	"Ninety",
	NULL
} ;


/* exported subroutines */


int ctwords(string *sp,int v)
{
	int		rs = SR_OK ;
	if (v > 0) {
	    while (v > 0) {
		if (v >= 1000000000) {
		    speak_billions(sp,v) ;
		    v = (v%1000000000) ;
		} else if (v >= 1000000) {
		    speak_millions(sp,v) ;
		    v = (v%1000000) ;
		} else if (v >= 1000) {
		    speak_thousands(sp,v) ;
		    v = (v%1000) ;
		} else {
		    speak_group(sp,v) ;
		    v = 0 ;
		}
	    } /* end while */
	} else {
	    sp->append(" ") ;
	    sp->append(ones[0]) ;
	}
	rs = sp->length() ;
	return rs ;
}
/* end subroutine (speak) */


/* local subroutines */


static int speak_billions(string *sp,int v)
{
	const int	lv = (v/1000000000) ;
	speak_group(sp,lv) ;
	sp->append(" Billion") ;
	return 0 ;
}
/* end subroutine (speak_nillions) */


static int speak_millions(string *sp,int v)
{
	const int	lv = (v/1000000) ;
	speak_group(sp,lv) ;
	sp->append(" Million") ;
	return 0 ;
}
/* end subroutine (speak_millions) */


static int speak_thousands(string *sp,int v)
{
	const int	lv = (v/1000) ;
	speak_group(sp,lv) ;
	sp->append(" Thousand") ;
	return 0 ;
}
/* end subroutine (speak_thousands) */


static int speak_group(string *sp,int v)
{
	while (v > 0) {
	    if (v >= 100) {
	        speak_hundreds(sp,v) ;
		v = (v%100) ;
	    } else if ((v >= 10) && (v < 20)) {
		speak_teens(sp,v) ;
		v = 0 ;
	    } else if (v >= 10) {
		speak_tens(sp,v) ;
		v = (v%10) ;
	    } else if (v > 0) {
		speak_ones(sp,v) ;
		v = 0 ;
	    }
	} /* end while */
	return 0 ;
}
/* end subroutine (speak_group) */


static int speak_hundreds(string *sp,int v)
{
	const int	h = (v/100) ;
	speak_ones(sp,h) ;
	sp->append(" Hundred") ;
	return 0 ;
}


static int speak_tens(string *sp,int v)
{
	const int	t = (v/10) ;
	sp->append(" ") ;
	sp->append(tens[t]) ;
	return 0 ;
}


static int speak_teens(string *sp,int v)
{
	const int	t = (v%10) ;
	sp->append(" ") ;
	sp->append(teens[t]) ;
	return 0 ;
}
/* end subroutine (speak_ones) */


static int speak_ones(string *sp,int v)
{
	sp->append(" ") ;
	sp->append(ones[v]) ;
	return 0 ;
}
/* end subroutine (speak_ones) */


