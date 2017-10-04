/* sncpyw */

/* concatenate strings */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

	= 1999-12-03, David A­D­ Morano
	This was updated to use 'strlcpy(3c)' when it was rumored to be coming
	as a new standard.  We are currently using our own implementation of
	that, but when it is supported by vendors this will all seemlessly
	transistion to using the vendor version.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine concatenates strings into a single resulting string.

	Notes:

	These sorts of subroutines ('sncpyw(3dam)', 'sncpy(3dam)', etc) are
	quite carefully crafted in order to avoid buffer overflow and segfaults
	as well as leaving the destination buffer NUL-terminated.  Please
	observe and be careful with any possible modifications.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>


/* external subroutines */


/* forward subroutines */

int		sncpyw(char *,int,int,...) ;


/* exported subroutines */


int sncpy1w(dp,dl,s1,s1len)
char		dp[] ;
int		dl ;
const char	s1[] ;
int		s1len ;
{
	return sncpyw(dp,dl,1,s1,s1len) ;
}
/* end subroutine (sncpy1w) */

int sncpy2w(dp,dl,s1,s2,s2len)
char		dp[] ;
int		dl ;
const char	s1[], s2[] ;
int		s2len ;
{
	return sncpyw(dp,dl,2,s1,s2,s2len) ;
}
/* end subroutine (sncpy2w) */

int sncpy3w(dp,dl,s1,s2,s3,s3len)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[] ;
int		s3len ;
{
	return sncpyw(dp,dl,3,s1,s2,s3,s3len) ;
}
/* end subroutine (sncpy3w) */

int sncpy4w(dp,dl,s1,s2,s3,s4,s4len)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[], s4[] ;
int		s4len ;
{
	return sncpyw(dp,dl,4,s1,s2,s3,s4,s4len) ;
}
/* end subroutine (sncpy4w) */

int sncpy5w(dp,dl,s1,s2,s3,s4,s5,s5len)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[], s4[], s5[] ;
int		s5len ;
{
	return sncpyw(dp,dl,5,s1,s2,s3,s4,s5,s5len) ;
}
/* end subroutine (sncpy5w) */

int sncpy6w(dp,dl,s1,s2,s3,s4,s5,s6,s6len)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[], s4[], s5[], s6[] ;
int		s6len ;
{
	return sncpyw(dp,dl,6,s1,s2,s3,s4,s5,s6,s6len) ;
}
/* end subroutine (sncpy6w) */


int sncpyw(char *dp,int dl,int n,...)
{
	int		rs = SR_OK ;
	char		*bp = dp ;

	if (dp == NULL) return SR_FAULT ;

	if (dl < 0) dl = (INT_MAX - 1) ;

	dp[0] = '\0' ;

	{
	    va_list	ap ;
	    int		rlen = (dl+1) ;
	    int		i ;
	    int		ml ;
	    const char	*sp ;
	    va_begin(ap,n) ;
	    for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {
	        sp = (const char *) va_arg(ap,char *) ;
	        if (i < (n-1)) {
	            ml = strlcpy(bp,sp,rlen) ;
	            if (ml < rlen) bp += ml ;
	        } else { /* emulate 'strlcpy(3c)' but w/ given length */
	            int		sl = (int) va_arg(ap,int) ;
		    ml = 0 ;
		    while ((ml < (rlen-1)) && sl-- && *sp) {
		        *bp++ = *sp++ ;
		        ml += 1 ;
		    }
		    *bp = '\0' ;
		    if ((sl != 0) && *sp) ml += 1 ; /* error condition */
	        }
	        if (ml < rlen) {
	            rlen -= ml ;
	        } else {
		    rs = SR_OVERFLOW ;
		}
	    } /* end for */
	    va_end(ap) ;
	}

	return (rs >= 0) ? (bp - dp) : rs ;
}
/* end subroutine (sncpyw) */


