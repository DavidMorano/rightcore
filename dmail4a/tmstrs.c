/* tmstrs */

/* TM structure strings processing */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */	
#define	CF_THREEYEAR	1		/* use RFC2822 3-digit years */


/* revision history:

	= 1999-05-01, David A­D­ Morano
	This was created along with the DATE object.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines provide support for converting various date strings to
        their TM-structure equivalents.

	Note on year calculations for MSG-type year strings:

        RFC2822 says that all three digit years should be interpreted as being
        added to 2000 to get the actual year. This would seem to be an
        unfortunate choice since the only way that three digit years were ever
        used was due to buggy software from the Y2K problem (the year 2000 was
        represented as 100 in some time-date software). Therefore a better
        interpretation of three digit years would have been to add it to 1900 in
        order to get the actual year, but who are we, just dumb software
        designers that make the world work! See the compile-time switch
        CF_THREEYEAR for our options on this.

        Also, RFC2822 says that two digit years less than (whatever) about 50
        should be interpreted as being added to 2000 to get the actual year,
        while two digit years greater than about that should be added to 1900 to
        get the actual year. Prior practice in UNIX® was to use the year 69 or
        70 for this purpose. Better compatibility is probably achieved using 69,
        but we go with 70 for avant-gauard reasons!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#ifndef	CENTURY_BASE
#define	CENTURY_BASE	19
#endif

#define	TWOCHARS(a,b)		(((a) << 8) + (b))

#ifndef	TOUPPER
#define	TOUPPER(c)		((c) & (~ 0x20))
#endif
#ifndef	TOLOWER
#define	TOLOWER(c)		((c) | 0x20)
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


/* day-of-week */
int tmstrsday(cchar *sp,int sl)
{
	int		rs = SR_INVALID ;

	if (sl < 0) sl = strlen(sp) ;

	if ((sl >= 1) && (sl <= 9)) {
	    switch (TWOCHARS(TOUPPER(sp[0]),TOLOWER(sp[1]))) {
	    case TWOCHARS('S', 'u'):
	        rs = 0 ;
	        break ;
	    case TWOCHARS('M', 'o'):
	        rs = 1 ;
	        break ;
	    case TWOCHARS('T', 'u'):
	        rs = 2 ;
	        break ;
	    case TWOCHARS('W', 'e'):
	        rs = 3 ;
	        break ;
	    case TWOCHARS('T', 'h'):
	        rs = 4 ;
	        break ;
	    case TWOCHARS('F', 'r'):
	        rs = 5 ;
	        break ;
	    case TWOCHARS('S', 'a'):
	        rs = 6 ;
	        break ;
	    default:
	        rs = SR_INVALID ;
	        break ;
	    } /* end switch */
	} /* end if (possible) */

	return rs ;
}
/* end subroutine (tmstrsday) */


int tmstrsmonth(cchar *sp,int sl)
{
	int		rs = SR_INVALID ;

	if (sl < 0) sl = strlen(sp) ;

	if (sl >= 3) {
	    switch (TWOCHARS(TOUPPER(sp[0]),TOLOWER(sp[1]))) {
	    case TWOCHARS('J', 'a'):
	        rs = 0 ;
	        break ;
	    case TWOCHARS('F', 'e'):
	        rs = 1 ;
	        break ;
	    case TWOCHARS('M', 'a'):		/* March - May */
	        rs = ((TOLOWER(sp[2]) == 'r') ? 2 : 4) ;
	        break ;
	    case TWOCHARS('A', 'p'):
	        rs = 3 ;
	        break ;
	    case TWOCHARS('J', 'u'):		/* June - July */
	        rs = ((TOLOWER(sp[2]) == 'n') ? 5 : 6) ;
	        break ;
	    case TWOCHARS('A', 'u'):
	        rs = 7 ;
	        break ;
	    case TWOCHARS('S', 'e'):
	        rs = 8 ;
	        break ;
	    case TWOCHARS('O', 'c'):
	        rs = 9 ;
	        break ;
	    case TWOCHARS('N', 'o'):
	        rs = 10 ;
	        break ;
	    case TWOCHARS('D', 'e'):
	        rs = 11 ;
	        break ;
	    default:
	        rs = SR_INVALID ;		/* bad month name */
	        break ;
	    } /* end switch */
	} /* end if (possible) */

	return rs ;
}
/* end subroutine (tmstrsmonth) */


/* calclate the year based on the number of digits given */
int tmstrsyear(cchar *sp,int sl)
{
	int		rs = SR_INVALID ;
	int		year = 0 ;

	if (sl < 0) sl = strlen(sp) ;

	if ((sl >= 1) && (sl <= 5)) {
	    if ((rs = cfdeci(sp,sl,&year)) >= 0) {
	        switch (sl) {
	        case 1:
	            year += 100 ;
	            break ;
	        case 2:
	            if (year < 70) year += 100 ;
	            break ;
	        case 3:
#if	CF_THREEYEAR
	            year += 100 ;
#else
	            if (year < 70)
	                year += 100 ;
#endif
	            break ;
	        case 4:
	        case 5:
	            year -= TM_YEAR_BASE ;
	            break ;
	        } /* end switch */
	    } /* end if (cfdec) */
	} /* end if (possible) */

#if	CF_DEBUGS
	debugprintf("tmz/tmstrsyear: ret rs=%d year=%d\n",rs,year) ;
#endif

	return (rs >= 0) ? year : rs ;
}
/* end subroutine (tmstrsyear) */


