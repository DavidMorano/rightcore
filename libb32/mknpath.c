/* mknpath */

/* make a file path from components */


/* revision history:

	= 2001-12-03, David A­D­ Morano
        This code was born out of frustration with cleaning up bad legacy code
        (of which there is quite a bit -- like almost all of it).

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a file path out of one or more path
	componets.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdarg.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */


/* external variables */


/* local strutures */


/* forward references */

int		mknpath(char *,int,int,...) ;


/* local variables */


/* exported subroutines */


int mknpath1(pbuf,plen,s1)
char		pbuf[] ;
int		plen ;
const char	s1[] ;
{
	return mknpath(pbuf,plen,1,s1) ;
}
/* end subroutine (mknpath1) */


int mknpath2(pbuf,plen,s1,s2)
char		pbuf[] ;
int		plen ;
const char	s1[], s2[] ;
{
	return mknpath(pbuf,plen,2,s1,s2) ;
}
/* end subroutine (mknpath2) */


int mknpath3(pbuf,plen,s1,s2,s3)
char		pbuf[] ;
int		plen ;
const char	s1[], s2[], s3[] ;
{
	return mknpath(pbuf,plen,3,s1,s2,s3) ;
}
/* end subroutine (mknpath3) */


int mknpath4(pbuf,plen,s1,s2,s3,s4)
char		pbuf[] ;
int		plen ;
const char	s1[], s2[], s3[], s4[] ;
{
	return mknpath(pbuf,plen,4,s1,s2,s3,s4) ;
}
/* end subroutine (mknpath4) */


int mknpath5(pbuf,plen,s1,s2,s3,s4,s5)
char		pbuf[] ;
int		plen ;
const char	s1[], s2[], s3[], s4[], s5[] ;
{
	return mknpath(pbuf,plen,5,s1,s2,s3,s4,s5) ;
}
/* end subroutine (mknpath5) */


int mknpath(char *pbuf,int plen,int n,...)
{
	int		rs = SR_OK ;
	int		ml = 0 ;
	char		*bp = pbuf ;

	if (pbuf == NULL) return SR_FAULT ;

	if (plen < 0) plen = MAXPATHLEN ;

	{
	    va_list	ap ;
	    int		rlen = (plen + 1) ;
	    int		i ;
	    const char	*sp ;
	    va_begin(ap,n) ;
	    for (i = 0 ; i < n ; i += 1) {

	        sp = (const char *) va_arg(ap,char *) ;

	        if ((i > 0) && ((bp == pbuf) || (bp[-1] != '/')) && 
	            (sp[0] != '\0') && (sp[0] != '/')) {

	            if (rlen > 1) {		/* must be at least '2' */
	                *bp++ = '/' ;
	                rlen -= 1 ;
		    } else {
	                rs = SR_NAMETOOLONG ;
		    }

	        } /* end if (needed a pathname separator) */

		if (rs >= 0) {
	            if ((ml = strlcpy(bp,sp,rlen)) < rlen) {
	        	bp += ml ;
	        	rlen -= ml ;
		    } else {
	                rs = SR_NAMETOOLONG ;
		    }
		}

		if (rs < 0) break ;
	    } /* end for */
	    va_end(ap) ;
	} /* end block */

	*bp = '\0' ; /* in case of overflow */
	return (rs >= 0) ? (bp - pbuf) : rs ;
}
/* end subroutine (mknpath) */


