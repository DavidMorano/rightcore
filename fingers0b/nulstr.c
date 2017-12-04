/* nulstr */

/* assert a NUL-terminated string */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module (NULSTR) provides support for creating NUL-terminated
        strings when only a counted string is available.


*******************************************************************************/


#define	NULSTR_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"nulstr.h"


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


int nulstr_start(NULSTR *ssp,cchar *sp,int sl,cchar **rpp)
{
	int		rs = SR_OK ;
	int		cl = 0 ;

	if (ssp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	ssp->as = NULL ;
	*rpp = sp ;
	if (sl >= 0) {
	    if (sp[sl] != '\0') {
	        if (sl > NULSTR_SHORTLEN) {
		    cchar	*cp ;
	            if ((rs = uc_mallocstrw(sp,sl,&cp)) >= 0) {
	                cl = (rs-1) ;
	                *rpp = cp ;
	                ssp->as = cp ;
	            }
	        } else {
	            *rpp = ssp->buf ;
	            cl = strwcpy(ssp->buf,sp,sl) - ssp->buf ;
	        } /* end if */
	    } else {
	        cl = sl ;
	    }
	} else {
	    cl = strlen(sp) ;
	}

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (nulstr_start) */


int nulstr_finish(NULSTR *ssp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ssp == NULL) return SR_FAULT ;

	if (ssp->as != NULL) {
	    rs1 = uc_free(ssp->as) ;
	    if (rs >= 0) rs = rs1 ;
	    ssp->as = NULL ;
	}

	ssp->buf[0] = '\0' ;
	return rs ;
}
/* end subroutine (nulstr_finish) */


