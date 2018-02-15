/* bprinter */

/* print a line without trailing white-space */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1992-03-01, David A­D­ Morano
	This subroutine was originally written.

	= 1998-09-01, David A­D­ Morano
        This subroutine was modified to remove white-space in front of trailing
        white-space also.

*/

/* Copyright © 1992,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is meant to work only on TROFF source language (of
	whatever sort).

        This subroutine removes trailing white-space from a line, and then
        prints it. But if the line ends up as having only soe punctuation left
        to it and that punctuation starts in the first column, then some
        non-printing blank character is stuffed at the fron of the line to
        prevent the punctuation from starting the line. In the case of a period,
        this is particularly important since that might falsely indicate that a
        TROFF macro is present.

	Synopsis:

	int bprinter(bfile *tfp,int f_bol,cchar *lp,int ll)

	Arguments:

	tfp		pointer to BFILE object
	f_bol		flag indicating Beginning-Of-Line or not
	lp		character buffer to print
	ll		length of buffer to print

	Returns:

	-		number of characters (bytes) printed 


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff)
#endif

#undef	NSPUNCTS
#define	NSPUNCTS	10		/* max number of puncts to store */


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char	puncts[] = ".,;!?:" ;


/* exported subroutines */


int bprinter(bfile *tfp,int f_bol,cchar *lp,int ll)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (ll > 0) {
	    const int	splen = NSPUNCTS ;
	    const int	sch = MKCHAR(lp[0]) ;
	    int		spl = 0 ;
	    int		f_preserve ;
	    int		f = FALSE ;
	    char	spuncts[NSPUNCTS+1] ;
	    f_preserve = ((strchr(puncts,sch)) != NULL) && f_bol ;
	    if (lp[ll-1] == '\n') ll -= 1 ;
	    while (ll && CHAR_ISWHITE(lp[ll-1])) ll -= 1 ;
	    while (ll && lp[0]) {
		int	ch = MKCHAR(lp[ll-1]) ;
		if (strchr(puncts,ch) != NULL) {
		    if (spl < splen) {
			int	i = (splen-1-spl++) ;
			spuncts[i] = ch ;
		    } else {
			f = TRUE ;
		    }
		} else if (! CHAR_ISWHITE(ch)) {
		    f = TRUE ;
		}
		if (f) break ;
		ll -= 1 ;
	    } /* end while */
	    if (ll > 0) {
		rs = bwrite(tfp,lp,ll) ;
		wlen += rs ;
	    }
	    if ((rs >= 0) && (spl > 0)) {
		if ((ll == 0) && (! f_preserve)) {
		    rs = bwrite(tfp,"\\&",-1) ;
		    wlen += rs ;
	        }
		if (rs >= 0) {
		    int	i = (splen-spl) ;
		    rs = bwrite(tfp,(spuncts+i),spl) ;
		    wlen += rs ;
		}
	    }
	} /* end if (non-zero-length line) */

	if (rs >= 0) {
	    rs = bputc(tfp,CH_LF) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bprinter) */


