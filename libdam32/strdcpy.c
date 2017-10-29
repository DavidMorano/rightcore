/* strdcpy */

/* concatenate strings */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-12-03, David A­D­ Morano
	This was updated to use 'strlcpy(3c)' when it was rumored to be coming
	as new standard.  We are currently using our own implementation of
	that, but when it is supported by vendors this will all seemlessly
	transistion to using the vendor version.

	= 2011-12-09, David A­D­ Morano
	I got rid of the 'strlcpy(3c)' usage.  It was never really needed
	anyway.  The code is certainly cleaner without it.  And I don't really
	think it is a whole lot slower either since the various string lengths
	used are usually fairly small.  Other subroutines have gotten rid of
	'strlcpy(3c)' also without any complaints.

*/

/* Copyright © 1998,1999,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine concatenates strings into a single resulting string.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdarg.h>
#include	<string.h>

#include	<vsystem.h>


/* external subroutines */


/* external variables */


/* local strutures */


/* forward references */

static char	*strdcpy(char *,int,int,...) ;


/* local variables */


/* exported subroutines */


char *strdcpy1(dp,dl,s1)
char		dp[] ;
int		dl ;
const char	s1[] ;
{

	return strdcpy(dp,dl,1,s1) ;
}
/* end subroutine (strdcpy1) */

char *strdcpy2(dp,dl,s1,s2)
char		dp[] ;
int		dl ;
const char	s1[], s2[] ;
{

	return strdcpy(dp,dl,2,s1,s2) ;
}
/* end subroutine (strdcpy2) */

char *strdcpy3(dp,dl,s1,s2,s3)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[] ;
{

	return strdcpy(dp,dl,3,s1,s2,s3) ;
}
/* end subroutine (strdcpy3) */

char *strdcpy4(dp,dl,s1,s2,s3,s4)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[], s4[] ;
{

	return strdcpy(dp,dl,4,s1,s2,s3,s4) ;
}
/* end subroutine (strdcpy4) */

char *strdcpy5(dp,dl,s1,s2,s3,s4,s5)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[], s4[], s5[] ;
{

	return strdcpy(dp,dl,5,s1,s2,s3,s4,s5) ;
}
/* end subroutine (strdcpy5) */

char *strdcpy6(dp,dl,s1,s2,s3,s4,s5,s6)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[], s4[], s5[], s6[] ;
{

	return strdcpy(dp,dl,6,s1,s2,s3,s4,s5,s6) ;
}
/* end subroutine (strdcpy6) */


/* local subroutines */


static char *strdcpy(char *dp,int dl,int n,...)
{
	if (dl < 0) dl = INT_MAX ;

	{
	    va_list	ap ;
	    int		i ;
	    const char	*sp ;
	    va_begin(ap,n) ;
	    for (i = 0 ; (dl > 0) && (i < n) ; i += 1) {
	        sp = (cchar *) va_arg(ap,char *) ;
	        while ((dl > 0) && (sp[0] != '\0')) {
		    *dp++ = *sp++ ;
		    dl -= 1 ;
	        }
	    } /* end for */
	    va_end(ap) ;
	}

	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strdcpy) */


