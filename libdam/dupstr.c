/* dupstr */

/* create a (writable) duplicate of a string */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module (DUPSTR) creates a writable duplicate of the given
        string.

        Note: for a simple non-writable (constant) copy of a string, check out
        the NULSTR object.

	Object:

	DUPSTR

	Methods:

	int dupstr_start(DUPSTR *ssp,cchar *sp,int sl,char **rpp)
	int dupstr_finish(DUPSTR *ssp)


*******************************************************************************/


#define	DUPSTR_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"dupstr.h"


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,cchar *,int) ;


/* exported subroutines */


int dupstr_start(DUPSTR *ssp,cchar *sp,int sl,char **rpp)
{
	int		rs = SR_OK ;
	int		cl = 0 ;

	if (ssp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	ssp->as = NULL ;
	if (sl < 0) sl = strlen(sp) ;

	if (sl > DUPSTR_SHORTLEN) {
	    const int	ssize = (sl+1) ;
	    char	*bp ;

	    if ((rs = uc_malloc(ssize,&bp)) >= 0) {
	        cl = strwcpy(bp,sp,sl) - bp ;
	        *rpp = bp ;
	        ssp->as = bp ;
	    } /* end if (m-a) */

	} else {

	    *rpp = ssp->buf ;
	    cl = strwcpy(ssp->buf,sp,sl) - ssp->buf ;

	} /* end if */

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (dupstr_start) */


int dupstr_finish(DUPSTR *ssp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ssp != NULL) {
	    if (ssp->as != NULL) {
	        rs1 = uc_free(ssp->as) ;
	        if (rs >= 0) rs = rs1 ;
	        ssp->as = NULL ;
	    }
	    ssp->buf[0] = '\0' ;
	} else {
	    rs = SR_FAULT ;
	}

	return rs ;
}
/* end subroutine (dupstr_finish) */


