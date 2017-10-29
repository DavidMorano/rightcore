/* compactstr */

/* compact the whitespace out of a string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-10, David A.D. Morano
	This was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine compacts a string -- in place -- by removing redundant
	whitespace.  Also non-blank whitespaces are replaced only by blanks.
	Also any leading whitespace is removed.

	Synopsis:

	int compactstr(sbuf,slen)
	char		sbuf[] ;
	int		slen ;

	Arguments:

	sbuf		source (and destination) string
	slen		length of source string

	Returns:

	<0		error
	>=0		resulting string length


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<estrings.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	hasdoublewhite(const char *,int) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy2(char *,int,const char *,const char *) ;
extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strdcpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnrpbrk(const char *,int,const char *) ;
extern char	*strwcpyblanks(char *,int) ;	/* NUL-terminaed */
extern char	*strncpyblanks(char *,int) ;	/* not NUL-terminated */
extern char	*strwset(char *,int,int) ;
extern char	*strnset(char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int compactstr(char *sbuf,int slen)
{
	int		bl ;

	if (slen < 0) slen = strlen(sbuf) ;

	bl = slen ;
	if ((slen > 0) && hasdoublewhite(sbuf,slen)) {
	    int		sl = slen ;
	    int		cl ;
	    const char	*sp = sbuf ;
	    const char	*cp ;
	    char	buf[slen+1] ;
	    char	*bp ;

	    bl = 0 ;
	    buf[0] = '\0' ;
	    while ((cl = nextfield(sp,sl,&cp)) > 0) {

		if (bl > 0)
		    buf[bl++] = ' ' ;

		bp = (buf + bl) ;
		bl += (strwcpy(bp,cp,cl) - bp) ;

		sl -= ((cp + cl) - sp) ;
		sp = (cp + cl) ;

	    } /* end while */

	    if (bl > 0)
		strwcpy(sbuf,buf,bl) ;

	} /* end if (hasdouble) */

	return bl ;
}
/* end subroutine (compactstr) */


