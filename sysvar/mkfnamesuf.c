/* mkfnamesuf */

/* make a file name from parts (one base and some suffixes) */


/* revision history:

	= 2001-12-03, David A­D­ Morano
	This was made specifically for the HDB UUCP modified code.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a filename (a single filename component) out
	of several possible strings.  The first string is a base name, the
	other possible strings are suffix components.

	Synopsis:

	int mkfnamesuf(ofname,n,p1,...)
	char		ofname[] ;
	int		n ;
	const char	p1[] ;
	...

	Arguments:

	ofname		buffer for the resulting filename
	n		number of suffixes
	p1		base file-name component
	...		one or more possible strings

	Returns:

	>=0		OK, length of resulting filename
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<storebuf.h>


/* forward references */

int	mkfnamesuf(char *,int,const char *,...) ;


/* exported subroutines */


int mkfnamesuf1(char *ofname,cchar *p1,cchar *s1)
{
	return mkfnamesuf(ofname,1,p1,s1) ;
}
/* end subroutine (mkfnamesuf1) */


int mkfnamesuf2(char *ofname,cchar *p1,cchar *s1,cchar *s2)
{
	return mkfnamesuf(ofname,2,p1,s1,s2) ;
}
/* end subroutine (mkfnamesuf2) */


int mkfnamesuf3(char *ofname,cchar *p1,cchar *s1,cchar *s2,cchar *s3)
{
	return mkfnamesuf(ofname,3,p1,s1,s2,s3) ;
}
/* end subroutine (mkfnamesuf3) */


int mkfnamesuf4(char *ofname,cchar *p1,cchar *s1,cchar *s2,cchar *s3,cchar *s4)
{
	return mkfnamesuf(ofname,4,p1,s1,s2,s3,s4) ;
}
/* end subroutine (mkfnamesuf4) */


int mkfnamesuf5(ofname,p1,s1,s2,s3,s4,s5)
char		ofname[] ;
const char	p1[] ;
const char	s1[], s2[], s3[], s4[],s5[] ;
{
	return mkfnamesuf(ofname,5,p1,s1,s2,s3,s4,s5) ;
}
/* end subroutine (mkfnamesuf5) */


int mkfnamesuf(char *rbuf,int n,const char *p1,...)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,rl,p1,-1) ;
	    rl += rs ;
	}

	if ((rs >= 0) && (n > 0)) {
	    rs = storebuf_char(rbuf,rlen,rl,'.') ;
	    rl += rs ;
	}

	{
	    va_list	ap ;
	    int		i ;
	    const char	*sp ;
	    va_begin(ap,p1) ;
	    for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {
	        sp = (const char *) va_arg(ap,const char *) ;
		if (sp != NULL) {
	            rs = storebuf_strw(rbuf,rlen,rl,sp,-1) ;
	            rl += rs ;
		}
	    } /* end for */
	    va_end(ap) ;
	}

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (mkfnamesuf) */


