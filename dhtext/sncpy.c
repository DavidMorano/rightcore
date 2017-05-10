/* sncpy */

/* concatenate strings */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-12-03, David A­D­ Morano
	This was updated to use 'strlcpy(3c)' when it was rumored to be coming
	as a new standard.  We are currently using our own implementation of
	that, but when it is supported by vendors this will all seemlessly
	transistion to using the vendor version.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine concatenates strings into a single resulting string.

	Observation 2015-09-29, David A­D­ Morano:

	Just some musing here: was the choice of changing to use |strlcpy(3c)|
	really such a good idea after all?  Just wondering.  Yes, we used it
	now for tons of years, but was it really faster than what we had
	before?  This goes diddo for the other places which were changed the
	code to use |strlcpy(3c)|.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */


/* external variables */


/* local strutures */


/* forward references */

int	sncpy(char *,int,int,...) ;


/* local variables */


/* exported subroutines */


int sncpy1(char *dp,int dl,cchar *s1)
{
	return sncpy(dp,dl,1,s1) ;
}
/* end subroutine (sncpy1) */

int sncpy2(char *dp,int dl,cchar *s1,cchar *s2)
{
	return sncpy(dp,dl,2,s1,s2) ;
}
/* end subroutine (sncpy2) */

int sncpy3(char *dp,int dl,cchar *s1,cchar *s2,cchar *s3)
{
	return sncpy(dp,dl,3,s1,s2,s3) ;
}
/* end subroutine (sncpy3) */

int sncpy4(char *dp,int dl,cchar *s1,cchar *s2,cchar *s3,cchar *s4)
{
	return sncpy(dp,dl,4,s1,s2,s3,s4) ;
}
/* end subroutine (sncpy4) */

int sncpy5(char *dp,int dl,cchar *s1,cchar *s2,cchar *s3,cchar *s4,cchar *s5)
{
	return sncpy(dp,dl,5,s1,s2,s3,s4,s5) ;
}
/* end subroutine (sncpy5) */

int sncpy6(dp,dl,s1,s2,s3,s4,s5,s6)
char		*dp ;
int		dl ;
const char	s1[], s2[], s3[], s4[], s5[], s6[] ;
{
	return sncpy(dp,dl,6,s1,s2,s3,s4,s5,s6) ;
}
/* end subroutine (sncpy6) */


int sncpy(char *dp,int dl,int n,...)
{
	int		rs = SR_OK ;
	char		*bp = dp ;

	if (dl < 0) dl = (INT_MAX - 1) ;

	dp[0] = '\0' ;

	{
	    va_list	ap ;
	    int		rlen = (dl+1) ;
	    int		i ;
	    int		ml ;
	    const char	*sp ;
	    va_begin(ap,n) ;
	    for (i = 0 ; i < n ; i += 1) {
	        sp = (const char *) va_arg(ap,const char *) ;
	        if ((ml = strlcpy(bp,sp,rlen)) >= rlen) {
	            rs = SR_OVERFLOW ;
		    break ;
	        }
	        bp += ml ;
	        rlen -= ml ;
	    } /* end for */
	    va_end(ap) ;
	} /* end block */

	return (rs >= 0) ? (bp - dp) : rs ;
}
/* end subroutine (sncpy) */


