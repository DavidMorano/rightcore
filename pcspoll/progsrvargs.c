/* progsrvargs */

/* process server file program arguments */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to parse the SERVER program arguments from an
        expanded (substituted) server file entry. Basically, we just
        "field-SHELL" out arguments and put them into the supplied vector-string
        object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<field.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */


/* externals variables */


/* forward references */


/* local structures */


/* exported subroutines */


int progsrvargs(PROGINFO *pip,vecstr *alp,cchar *abuf)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
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

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progsrvargs) */


