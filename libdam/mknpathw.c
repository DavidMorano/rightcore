/* mknpathw */

/* make a file path from components */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-12-03, David A­D­ Morano
	This code was born out of frustration with cleaning up bad legacy code
	(of which there is quite a bit -- like almost all of it).

	= 2011-12-09, David A­D­ Morano
	I got rid of the 'strlcpy(3c)' usage.  That subroutine just does not
	represent my moral values!  I now do not know what prompted me to do
	this (probably its extra complexity to use).

*/

/* Copyright © 2001,1011 David A­D­ Morano.  All rights reserved. */

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


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local strutures */


/* forward references */

int		mknpathw(char *,int,int,...) ;


/* local variables */


/* exported subroutines */


int mknpath1w(pbuf,plen,s1,s1len)
char		pbuf[] ;
int		plen ;
const char	s1[] ;
int		s1len ;
{
	return mknpathw(pbuf,plen,1,s1,s1len) ;
}
/* end subroutine (mknpath1w) */


int mknpath2w(pbuf,plen,s1,s2,s2len)
char		pbuf[] ;
int		plen ;
const char	s1[], s2[] ;
int		s2len ;
{
	return mknpathw(pbuf,plen,2,s1,s2,s2len) ;
}
/* end subroutine (mknpath2w) */


int mknpath3w(pbuf,plen,s1,s2,s3,s3len)
char		pbuf[] ;
int		plen ;
const char	s1[], s2[], s3[] ;
int		s3len ;
{
	return mknpathw(pbuf,plen,3,s1,s2,s3,s3len) ;
}
/* end subroutine (mknpath3w) */


int mknpath4w(pbuf,plen,s1,s2,s3,s4,s4len)
char		pbuf[] ;
int		plen ;
const char	s1[], s2[], s3[], s4[] ;
int		s4len ;
{
	return mknpathw(pbuf,plen,4,s1,s2,s3,s4,s4len) ;
}
/* end subroutine (mknpath4w) */


int mknpath5w(pbuf,plen,s1,s2,s3,s4,s5,s5len)
char		pbuf[] ;
int		plen ;
const char	s1[], s2[], s3[], s4[], s5[] ;
int		s5len ;
{
	return mknpathw(pbuf,plen,5,s1,s2,s3,s4,s5,s5len) ;
}
/* end subroutine (mknpath5w) */


int mknpathw(char *pbuf,int plen,int n,...)
{
	int		rs = SR_OK ;
	int		bl ;
	char		*bp = pbuf ;

	if (pbuf == NULL) return SR_FAULT ;

	if (plen < 0) plen = MAXPATHLEN ;

	bl = plen ;

	{
	    va_list	ap ;
	    int		i ;
	    int		sl = -1 ;
	    const char	*sp ;
	    va_begin(ap,n) ;
	    for (i = 0 ; i < n ; i += 1) {

	        sp = (const char *) va_arg(ap,char *) ;

	        if (i == (n-1)) sl = (int) va_arg(ap,int) ;

	        if ((i > 0) && ((bp == pbuf) || (bp[-1] != '/')) && 
	            (sp[0] != '\0') && (sp[0] != '/')) {

	            if (bl > 0) {
	                *bp++ = '/' ;
	                bl -= 1 ;
	            } else
	                rs = SR_NAMETOOLONG ;

	        } /* end if (needed a pathname separator) */

	        if (rs >= 0) {
	            if ((rs = snwcpy(bp,bl,sp,sl)) >= 0) {
	                bp += rs ;
	                bl -= rs ;
	            } else if (rs == SR_OVERFLOW)
	                rs = SR_NAMETOOLONG ;
	        }

		if (rs < 0) break ;
	    } /* end for */
	    va_end(ap) ;
	} /* end block */

	*bp = '\0' ; /* in case of overflow */
	return (rs >= 0) ? (bp - pbuf) : rs ;
}
/* end subroutine (mknpathw) */


