/* vecstr_srvargs */

/* process server file program arguments */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* run-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

	= 2017-11-30, David A­D­ Morano
	This code has been borrowed from the above.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to parse the server program arguments from an
        expanded (substituted) service file entry. Basically, we just
        "field-SHELL" out arguments and put them into the supplied vector-string
        object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<field.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* externals variables */


/* forward references */


/* local structures */


/* exported subroutines */


int vecstr_srvargs(vecstr *alp,cchar *abuf)
{
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("progsrvargs: ent a=>%s<\n",abuf) ;
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
	                c += 1 ;
	                rs = vecstr_add(alp,fbuf,fl) ;
	                if (rs < 0) break ;
	            } /* end while */
		    uc_free(fbuf) ;
		} /* end if (m-a) */
	        field_finish(&fsb) ;
	    } /* end if (field) */

	} /* end if (non-null non-zero) */

#if	CF_DEBUGS
	debugprintf("progsrvargs: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_srvargs) */


