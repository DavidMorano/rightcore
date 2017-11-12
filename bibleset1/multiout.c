/* multiout */

/* output multiple stuff w/ an object for convenience */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2009-04-01, David A­D­ Morano
        This subroutine was written as an enhancement for adding back-matter
        (end pages) to the output document.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine does something! :-)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"multiout.h"


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mo_start(mop,ofp)
MULTIOUT	 *mop ;
bfile		*ofp ;
{

	if (mop == NULL) return SR_FAULT ;
	if (ofp == NULL) return SR_FAULT ;

	memset(mop,0,sizeof(MULTIOUT	)) ;

	mop->ofp = ofp ;
	return SR_OK ;
}
/* end subroutine (mo_start) */


int mo_printf(MULTIOUT *mop,const char *fmt,...)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (mop == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	rs = mop->rs ;
	if (rs >= 0) {
	    {
		va_list	ap ;
		va_begin(ap,fmt) ;
		rs = bvprintf(mop->ofp,fmt,ap) ;
		wlen = rs ;
		va_end(ap) ;
	    }
	    mop->rs = rs ;
	    mop->wlen += wlen ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mo_printf) */


int mo_finish(mop)
MULTIOUT	 *mop ;
{
	int		rs ;
	int		wlen = 0 ;

	if (mop == NULL) return SR_FAULT ;

	rs = mop->rs ;
	wlen = mop->wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mo_finish) */


