/* vecstr_svcargs */

/* load service arguments */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine loads service arguments (arguments which accompany
	a service string) into a VECSTR object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<field.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	strwcasecmp(cchar *,cchar *,int) ;


/* externals variables */


/* forward references */


/* local structures */


/* exported subroutines */


int vecstr_svcargs(vecstr *op,cchar *abuf)
{
	int		rs = SR_OK ;
	int		f = 0 ;

#if	CF_DEBUGS
	debugprintf("vecstr_svcargs: ent a=>%s<\n",abuf) ;
#endif

	if ((abuf != NULL) && (abuf[0] != '\0')) {
	    FIELD	fsb ;
	    const int	alen = strlen(abuf) ;
	    uchar	terms[32] ;

	    fieldterms(terms,0," \t") ;

	    if ((rs = field_start(&fsb,abuf,alen)) >= 0) {
	        const int	flen = alen ;
	        char		*fbuf ;
		if ((rs = uc_malloc((flen+1),&fbuf)) >= 0) {
	            int		fl ;
	            while ((fl = field_sharg(&fsb,terms,fbuf,flen)) >= 0) {
			if (strwcasecmp("/W",fbuf,fl) == 0) {
			    f = TRUE ;
			} else {
	                    rs = vecstr_add(op,fbuf,fl) ;
			}
	                if (rs < 0) break ;
	            } /* end while */
		    uc_free(fbuf) ;
		} /* end if (m-a) */
	        field_finish(&fsb) ;
	    } /* end if (field) */

	} /* end if (non-null non-zero) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (vecstr_svcargs) */


