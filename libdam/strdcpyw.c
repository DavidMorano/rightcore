/* strdcpyw */

/* concatenate strings */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-12-03, David A­D­ Morano
        This was updated to use 'strlcpy(3c)' when it was rumored to be coming.
        We are using our own version of 'strlcpy(3c)' before it is provided by
        vendors.

	= 2011-12-09, David A­D­ Morano
        I got rid of the 'strlcpy(3c)' usage. It was never really needed anyway.
        The code is certainly cleaner without it. And I don't really think it is
        a whole lot slower either since the various string lengths used are
        usually fairly small. Other subroutines have gotten rid of 'strlcpy(3c)'
        also without any complaints.

*/

/* Copyright © 1998,1999,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine concatenates strings into a single resulting string.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>


/* external subroutines */


/* external variables */


/* local strutures */


/* forward references */

static char	*strdcpyw(char *,int,int,...) ;


/* local variables */


/* exported subroutines */


char *strdcpy1w(dp,dl,s1,s1len)
char		dp[] ;
int		dl ;
const char	s1[] ;
int		s1len ;
{

	return strdcpyw(dp,dl,1,s1,s1len) ;
}
/* end subroutine (strdcpy1w) */

char *strdcpy2w(dp,dl,s1,s2,s2len)
char		dp[] ;
int		dl ;
const char	s1[], s2[] ;
int		s2len ;
{

	return strdcpyw(dp,dl,2,s1,s2,s2len) ;
}
/* end subroutine (strdcpy2w) */

char *strdcpy3w(dp,dl,s1,s2,s3,s3len)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[] ;
int		s3len ;
{

	return strdcpyw(dp,dl,3,s1,s2,s3,s3len) ;
}
/* end subroutine (strdcpy3w) */

char *strdcpy4w(dp,dl,s1,s2,s3,s4,s4len)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[], s4[] ;
int		s4len ;
{

	return strdcpyw(dp,dl,4,s1,s2,s3,s4,s4len) ;
}
/* end subroutine (strdcpy4w) */

char *strdcpy5w(dp,dl,s1,s2,s3,s4,s5,s5len)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[], s4[], s5[] ;
int		s5len ;
{

	return strdcpyw(dp,dl,5,s1,s2,s3,s4,s5,s5len) ;
}
/* end subroutine (strdcpy5w) */

char *strdcpy6w(dp,dl,s1,s2,s3,s4,s5,s6,s6len)
char		dp[] ;
int		dl ;
const char	s1[], s2[], s3[], s4[], s5[], s6[] ;
int		s6len ;
{

	return strdcpyw(dp,dl,6,s1,s2,s3,s4,s5,s6,s6len) ;
}
/* end subroutine (strdcpy6w) */


/* local subroutines */


static char *strdcpyw(char *dp,int dl,int n,...)
{
	if (dl < 0) dl = INT_MAX ;

	{
	    va_list	ap ;
	    int		i ;
	    const char	*sp ;
	    va_begin(ap,n) ;
	    for (i = 0 ; (dl > 0) && (i < n) ; i += 1) {
	        sp = (const char *) va_arg(ap,char *) ;
	        if (i == (n-1)) {
	            int	sl = (int) va_arg(ap,int) ;
	            while ((dl > 0) && sl && (sp[0] != '\0')) {
		        *dp++ = *sp++ ;
		        dl -= 1 ;
		        sl -= 1 ;
	            }
	        } else {
	            while ((dl > 0) && (sp[0] != '\0')) {
		        *dp++ = *sp++ ;
		        dl -= 1 ;
	            }
	        }
	    } /* end for */
	    va_end(ap) ;
	}

	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strdcpyw) */


