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

	This subroutine removes trailing white-space from a line,
	and then prints it.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

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


int bprinter(tfp,f_bol,lp,ll)
bfile		*tfp ;
const char	*lp ;
int		ll ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (ll > 0) {
	    const int	splen = NSPUNCTS ;
	    int		spl = 0 ;
	    int		f_preserve ;
	    char	spuncts[NSPUNCTS+1] ;
	    f_preserve = ((strchr(puncts,MKCHAR(lp[0])) != NULL) && f_bol) ;
	    if (lp[ll-1] == '\n') ll -= 1 ;
	    while (ll && CHAR_ISWHITE(lp[ll-1])) ll -= 1 ;
	    while (ll && lp[0]) {
		int	ch = MKCHAR(lp[ll-1]) ;
		if (strchr(puncts,ch) != NULL) {
		    if (spl < splen) {
			int	i = (splen-1-spl++) ;
			spuncts[i] = ch ;
		    } else
			break ;
		} else if (! CHAR_ISWHITE(ch)) 
		    break ;
		ll -= 1 ;
	    } /* end while */
	    if (ll > 0) {
		rs = bwrite(tfp,lp,ll) ;
		wlen += rs ;
	    }
	    if ((rs >= 0) && (spl > 0)) {
		if ((ll == 0) && (! f_preserve)) {
		    rs = bwrite(tfp,"\\%",-1) ;
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



