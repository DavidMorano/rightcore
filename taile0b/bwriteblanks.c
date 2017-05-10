/* bwriteblanks */

/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_TESTLINE	0		/* test long lines */
#define	CF_MEMSET	0		/* use 'memset(3c)' */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine just prints out some blank characters.


*******************************************************************************/

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#if	CF_TESTLINE && CF_DEBUGS
#undef	LINEBUFLEN
#define	LINEBUFLEN	20
#else
#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif
#endif

#define	NBLANKS		32


/* external subroutines */

extern int	strwset(char *,int,int) ;
extern int	strnset(char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int bwriteblanks(bfile *ofp,int n)
{
	int		rs = SR_OK ;
	int		blen = n ;
	int		mlen ;
	int		wlen = 0 ;
	char		blanks[NBLANKS+1] ;

	if (n < 0) return SR_DOM ;

	mlen = MIN(NBLANKS,n) ;
#if	CF_MEMSET
	memset(blanks,' ',mlen) ;
#else
	strnset(blanks,' ',mlen) ;
#endif /* CF_MEMSET */

	while ((rs >= 0) && (blen > 0)) {
	    mlen = MIN(blen,NBLANKS) ;
	    rs = bwrite(ofp,blanks,mlen) ;
	    wlen += rs ;
	    blen -= mlen ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bwriteblanks) */


