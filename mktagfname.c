/* mktagfname */

/* make a tag filename */


#define	CF_DEBUGS	0		/* used for little object below */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine makes (creates) a tag filename given a tag basename and
	a base directory name.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mktagfname(fname,basedname,sp,sl)
char		fname[] ;
const char	basedname[] ;
const char	sp[] ;
int		sl ;
{
	const int	maxpl = MAXPATHLEN ;
	int		rs = SR_OK ;

	if (fname == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

/* initialize */

	fname[0] = '\0' ;

/* get the full tag filename */

	if ((sp[0] != '/') && (basedname != NULL)) {
	    int	i = 0 ;

	    if (rs >= 0) {
	        rs = storebuf_strw(fname,maxpl,i,basedname,-1) ;
	        i += rs ;
	    }

	    if (rs >= 0) {
		while ((i > 0) && (fname[i - 1] == '/')) {
		    i -= 1 ;
		}
	    }

	    if (rs >= 0) {
	        rs = storebuf_char(fname,maxpl,i,'/') ;
	        i += rs ;
	    }

	    if (rs >= 0) {
	        rs = storebuf_strw(fname,maxpl,i,sp,sl) ;
	        i += rs ;
	    }

	    if (rs >= 0)
		rs = i ;

	} else
	    rs = snwcpy(fname,maxpl,sp,sl) ;

	return rs ;
}
/* end subroutine (mktagfname) */


