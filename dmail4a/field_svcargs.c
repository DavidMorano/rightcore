/* field_svcargs */

/* parse a FIELD object into service envelope items */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will take a 'FIELD' object and creates a string-list of
        service arguments from it.

	Synopsis:

	int field_svcargs(fbp,sap)
	FIELD		*fbp ;
	VECSTR		*sap ;

	Arguments:

	fbp		pointer to FIELD object
	sap		pointer to VECSTR of arguments

	Returns:

	>=0		OK
	<0		bad


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<field.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int field_svcargs(FIELD *fbp,vecstr *sap)
{
	int		rs ;
	int		c = 0 ;

	if (fbp == NULL) return SR_FAULT ;
	if (sap == NULL) return SR_FAULT ;

	if ((rs = field_remaining(fbp,NULL)) >= 0) {
	    const int	alen = rs ;
	    char	*abuf ;
	    if ((rs = uc_malloc((alen+1),&abuf)) >= 0) {
		int	al ;
	        while ((al = field_sharg(fbp,NULL,abuf,alen)) >= 0) {
	            c += 1 ;
	            rs = vecstr_add(sap,abuf,al) ;
	            if (rs < 0) break ;
	        } /* end while */
	        uc_free(abuf) ;
	    } /* end if (m-a) */
	} /* end if (field_remaining) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (field_svcargs) */


