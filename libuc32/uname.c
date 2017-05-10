/* uname */

/* UNIX® information (a cache for 'uname(2)' ) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module serves as a cache (of sorts) for UNIX® information that is
        related to the underlying machine and which does not (easily) change
        during program exection.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"uname.h"


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uname_start(UNAME *op)
{
	const int	size = sizeof(struct utsname) ;
	int		rs ;
	void		*p ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = uc_malloc(size,&p)) >= 0) {
	    struct utsname	*unp = p ;
	    if ((rs = u_uname(unp)) >= 0) {
	        const int	nlen = NODENAMELEN ;
	        int		size = 0 ;
	        char		*bp ;
	        size += (strnlen(unp->sysname,nlen) + 1) ;
	        size += (strnlen(unp->nodename,nlen) + 1) ;
	        size += (strnlen(unp->release,nlen) + 1) ;
	        size += (strnlen(unp->version,nlen) + 1) ;
	        size += (strnlen(unp->machine,nlen) + 1) ;
	        if ((rs = uc_malloc(size,&bp)) >= 0) {
	            op->a = bp ;
	            op->sysname = bp ;
	            bp = (strwcpy(bp,unp->sysname,nlen) + 1) ;
	            op->nodename = bp ;
	            bp = (strwcpy(bp,unp->nodename,nlen) + 1) ;
	            op->release = bp ;
	            bp = (strwcpy(bp,unp->release,nlen) + 1) ;
	            op->version = bp ;
	            bp = (strwcpy(bp,unp->version,nlen) + 1) ;
	            op->machine = bp ;
	            bp = (strwcpy(bp,unp->machine,nlen) + 1) ;
	        } /* end if (memory-allocation) */
	    } /* end if (uname) */
	    uc_free(unp) ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (uname_start) */


int uname_finish(UNAME *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}

	return rs ;
}
/* end subroutine (uname_finish) */


