/* main (numwords) */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2010-08-23, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2010 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<cstdlib>
#include	<cinttypes>
#include	<cstring>
#include	<new>
#include	<algorithm>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	VARDEBUGFNAME	"NUMWORDS_DEBUGFILE" 


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" int	sisub(cchar *,int,cchar *) ;
extern "C" int	cfdeci(cchar *,int,int *) ;
extern "C" int	mkrevstr(char *,int) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* global variables */


/* local structures (and methods) */


/* forward references */

static int speak(string *,int) ;
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


int main(int argc,const char **argv,const char **envv)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting DFD=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */
	if (argc > 1) {
	    int	ai ;
	    for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
		int	v ;
		cchar	*a = argv[ai] ;
		if ((rs = cfdeci(a,-1,&v)) >= 0) {
		    string	s ;
		    rs = speak(&s,v) ;
		    cout << s << endl ;
		}
		if (rs < 0) break ;
	    } /* end for */
	}
#if	CF_DEBUGS
	debugclose() ;
#endif
	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int speak(string *sp,int v)
{
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
	return 0 ;
}
/* end subroutine (speak) */


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


