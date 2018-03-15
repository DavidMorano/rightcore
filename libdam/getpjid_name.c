/* getpjid_name */

/* get the project-ID for a given project-name */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This was written to collect this code into one subroutine.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns a project-ID for a specified program-name. 

	Synopsis:

	int getpjid_name(cchar *np,int nl)

	Arguments:

	np		name of project to look up 
	nl		length of name (in bytes)

	Returns:

	<0		error
	>=0		PJID of given project name


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<vsystem.h>
#include	<getbufsize.h>
#include	<nulstr.h>
#include	<getax.h>
#include	<cfdec.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	hasalldig(cchar *,int) ;


/* exported subroutines */


int getpjid_name(cchar *np,int nl)
{
	NULSTR		n ;
	int		rs ;
	int		rs1 ;
	int		pjid = 0 ;
	cchar		*name ;
	if (np == NULL) return SR_FAULT ;
	if (np[0] == '\0') return SR_INVALID ;
	if ((rs = nulstr_start(&n,np,nl,&name)) >= 0) {
	    if ((rs = getbufsize(getbufsize_pj)) >= 0) {
	        struct project	pj ;
	        const int	pjlen = rs ;
	        char		*pjbuf ;
	        if ((rs = uc_malloc((pjlen+1),&pjbuf)) >= 0) {
		    {
	                rs = getpj_name(&pj,pjbuf,pjlen,name) ;
	                pjid = pj.pj_projid ;
		    }
	            rs1 = uc_free(pjbuf) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (memory-allocation) */
	    } /* end if (getbufsize) */
	    rs1 = nulstr_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */
	return (rs >= 0) ? pjid : rs ;
}
/* end subroutine (getpjid_name) */


int getpjid_proj(cchar *np,int nl)
{
	int		rs ;
	if (np == NULL) return SR_FAULT ;
	if (np[0] == '\0') return SR_INVALID ;
	if (hasalldig(np,nl)) {
	    int	v ;
	    if ((rs = cfdeci(np,nl,&v)) >= 0) {
		rs = v ;
	    }
	} else {
	    rs = getpjid_name(np,nl) ;
	}
	return rs ;
}
/* end subroutine (getpjid_proj) */


