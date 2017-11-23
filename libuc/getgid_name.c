/* getgid_name */

/* get the GID for a given group-name */


#define	CF_GETGROUP	0		/* compile in |getgroup_gid()| */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This was written to collect this code into one subroutine.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns a GID for a specified group-name. 

	Synopsis:

	int getgid_name(cchar *np,int nl)

	Arguments:

	np		name of group to look up 
	nl		length of given string

	Returns:

	<0		error
	>=0		GID of given group name


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


int getgid_name(cchar *np,int nl)
{
	NULSTR		n ;
	int		rs ;
	int		rs1 ;
	int		gid = 0 ;
	cchar		*name ;
	if (np == NULL) return SR_FAULT ;
	if (np[0] == '\0') return SR_INVALID ;
	if ((rs = nulstr_start(&n,np,nl,&name)) >= 0) {
	    struct group	gr ;
	    const int		grlen = getbufsize(getbufsize_gr) ;
	    char		*grbuf ;
	    if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
		{
	            rs = getgr_name(&gr,grbuf,grlen,name) ;
	            gid = gr.gr_gid ;
		}
	        rs1 = uc_free(grbuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (memory-allocation) */
	    rs1 = nulstr_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */
	return (rs >= 0) ? gid : rs ;
}
/* end subroutine (getgid_name) */


int getgid_group(cchar *np,int nl)
{
	int		rs ;
	if (np == NULL) return SR_FAULT ;
	if (np[0] == '\0') return SR_INVALID ;
	if (hasalldig(np,nl)) {
	    int		v ;
	    if ((rs = cfdeci(np,nl,&v)) >= 0) {
		rs = v ;
	    }
	} else {
	    rs = getgid_name(np,nl) ;
	}
	return rs ;
}
/* end subroutine (getgid_group) */


#if	CF_GETGROUP
int getgroup_gid(cchar *np,int nl)
{
	return getgid_group(np,nl) ;
}
#endif /* CF_GETGROUP */


