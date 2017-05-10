/* strdfill */

/* concatenate strings for target insertion */


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
	are usually fairly small.  Other subroutines have gotten rid of
	'strlcpy(3c)' also without any complaints.

*/

/* Copyright © 1998,1999,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a single string from one or more specificed
	strings and inserts it into the destination (not NUL-terminated).


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

static char	*strdfill(char *,int,int,...) ;


/* local variables */


/* exported subroutines */


char *strdfill1(bp,bl,s1)
char		*bp ;
int		bl ;
const char	s1[] ;
{
	return strdfill(bp,bl,1,s1) ;
}
/* end subroutine (strdfill1) */


char *strdfill2(bp,bl,s1,s2)
char		*bp ;
int		bl ;
const char	s1[], s2[] ;
{
	return strdfill(bp,bl,2,s1,s2) ;
}
/* end subroutine (strdfill2) */


char *strdfill3(bp,bl,s1,s2,s3)
char		*bp ;
int		bl ;
const char	s1[], s2[], s3[] ;
{
	return strdfill(bp,bl,3,s1,s2,s3) ;
}
/* end subroutine (strdfill3) */


char *strdfill4(bp,bl,s1,s2,s3,s4)
char		*bp ;
int		bl ;
const char	s1[], s2[], s3[], s4[] ;
{
	return strdfill(bp,bl,4,s1,s2,s3,s4) ;
}
/* end subroutine (strdfill4) */


char *strdfill5(bp,bl,s1,s2,s3,s4,s5)
char		*bp ;
int		bl ;
const char	s1[], s2[], s3[], s4[], s5[] ;
{
	return strdfill(bp,bl,5,s1,s2,s3,s4,s5) ;
}
/* end subroutine (strdfill5) */


char *strdfill6(bp,bl,s1,s2,s3,s4,s5,s6)
char		*bp ;
int		bl ;
const char	s1[], s2[], s3[], s4[], s5[], s6[] ;
{
	return strdfill(bp,bl,6,s1,s2,s3,s4,s5,s6) ;
}
/* end subroutine (strdfill6) */


/* local subroutines */


static char *strdfill(char *dp,int dl,int n,...)
{

	if (dl < 0) dl = INT_MAX ;

	{
	    va_list	ap ;
	    int		i ;
	    const char	*sp ;
	    va_begin(ap,n) ;
	    for (i = 0 ; (dl > 0) && (i < n) ; i += 1) {
	        sp = (const char *) va_arg(ap,char *) ;
	        while ((dl > 0) && (sp[0] != '\0')) {
		    *dp++ = *sp++ ;
		    dl -= 1 ;
	        } /* end while */
	    } /* end for */
	    va_end(ap) ;
	} /* end block */

	return dp ;
}
/* end subroutine (strdfill) */


