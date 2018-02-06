/* tabexpand */

/* expand TAB characters to the appropriate number of spaces */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-10 David A.D. Morano
	This was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine expands TAB characters in the given string into SPACE
        characters, as appropriate.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<char.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	NTABCOLS
#define	NTABCOLS	8		/* number of columns per tab */
#endif

#define	DSTORE		struct dstore


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	isprintlatin(int) ;
extern int	tabcols(int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy2(char *,int,const char *,const char *) ;
extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strdcpy4(char *,int,const char *,const char *,
			const char *,const char *) ;
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

struct dstore {
	char		*dp ;
	int		dl ;
	int		dlen ;
} ;


/* forward references */

static int	dstore_start(DSTORE *,char *,int) ;
static int	dstore_add(DSTORE *,int) ;
static int	dstore_finish(DSTORE *) ;


/* local variables */


/* exported subroutines */


int tabexpand(char *dbuf,int dlen,cchar *sbuf,int slen)
{
	DSTORE	d ;
	int		rs ;
	int		n, j ;
	int		sl = slen ;
	int		dcol = 0 ;
	const char	*sp = sbuf ;

	if ((rs = dstore_start(&d,dbuf,dlen)) >= 0) {

	    while ((dcol >= 0) && sl && sp[0]) {

	        if (*sp == '\t') {
	            n = tabcols(NTABCOLS,dcol) ;
	            for (j = 0 ; (dcol >= 0) && (j < n) ; j += 1)
	                dcol = dstore_add(&d,' ') ;
	        } else {
	            dcol = dstore_add(&d,*sp) ;
	        }

	        sp += 1 ;
	        sl -= 1 ;

	    } /* end while */

	    dcol = dstore_finish(&d) ;
	    if (rs >= 0) rs = dcol ;
	} /* end if (dstore) */

	return (rs >= 0) ? dcol : rs ;
}
/* end subroutine (tabexpand) */


/* local subroutines */


static int dstore_start(DSTORE *dp,char *dbuf,int dlen)
{

	dp->dlen = dlen ;
	dp->dp = dbuf ;
	dp->dl = 0 ;
	return SR_OK ;
}
/* end subroutine (dstore_start) */


static int dstore_add(DSTORE *dp,int c)
{
	int		rs = SR_OVERFLOW ;

	if (dp->dl < dp->dlen) {
	    *(dp->dp)++ = c ;
	    dp->dl += 1 ;
	    rs = dp->dl ;
	}

	return rs ;
}
/* end subroutine (dstore_add) */


static int dstore_finish(DSTORE *dp)
{

	dp->dp = '\0' ;
	return dp->dl ;
}
/* end subroutine (dstore_finish) */


