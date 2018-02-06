/* ids */

/* load up the process identification information (IDs) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will load up the various process user-group IDs, and the
        supplementary group IDs.

	Synopsis:

	int ids_load(IDS *op)

	Arguments:

	op		pointer to object

	Returns:

	<0		error
	>=0		number of group IDs returned


	Notes:

	We allocate dynamically rather than use NGROUPS_MAX.


*******************************************************************************/


#define	IDS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<unistd.h>		/* for |getgroups(2)| */
#include	<vsystem.h>
#include	<localmisc.h>
#include	"ids.h"


/* local defines */

#define	IDS_RESERVE	struct ids_reserve


/* external subroutines */


/* external variables */


/* local structures */

struct ids_reserve {
	int		ngroups ;
} ;


/* forward references */

int 		ids_init() ;

static int	ids_ngids(IDS *) ;


/* local variables */

static struct ids_reserve	ids_data ; /* zero-initialized */


/* exported subroutines */


/* special case of returning number of configured groups */
int ids_init()
{
	IDS_RESERVE	*irp = &ids_data ;
	int		rs ;
	if (irp->ngroups == 0) {
	    gid_t	dummy[1] ;
	    rs = u_getgroups(0,dummy) ;
	    irp->ngroups = rs ;
	} else {
	    rs = irp->ngroups ;
	}
	return rs ;
}
/* end subroutine (ids_init) */


/* must already be initially called to initialize object */
int ids_load(IDS *op)
{
	int		rs ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;

	op->gids = NULL ;
	op->uid = getuid() ;

	op->euid = geteuid() ;

	op->gid = getgid() ;

	op->egid = getegid() ;

	if ((rs = ids_init()) >= 0) {
	    const int	size = ((rs+1)*sizeof(gid_t)) ;
	    void	*p ;
	    n = rs ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
		op->gids = p ;
		if ((rs = u_getgroups(n,op->gids)) >= 0) {
		    op->gids[n] = -1 ;
		}
		if (rs < 0) {
		    uc_free(op->gids) ;
		    op->gids = NULL ;
		}
	    } /* end if (m-a) */
	} /* end if (ids_init) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ids_load) */


/* |ids_load()| must have already been called */
int ids_release(IDS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op == NULL) return SR_FAULT ;
	if (op->gids != NULL) {
	    rs1 = uc_free(op->gids) ;
	    if (rs >= 0) rs = rs1 ;
	    op->gids = NULL ;
	}
	return rs ;
}
/* end subroutine (ids_release) */


/* |ids_load()| must have already been called */
int ids_ngroups(IDS *op)
{
	int		n = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->gids != NULL) {
	    for (n = 0 ; op->gids[n] >= 0 ; n += 1) ;
	}
	return n ;
}
/* end subroutine (ids_ngroups) */


/* |ids_load()| must have already been called */
int ids_refresh(IDS *op)
{
	IDS_RESERVE	*irp = &ids_data ;
	int		rs = SR_OK ;
	int		rs1 ;
	irp->ngroups = 0 ;
	if (op->gids != NULL) {
	    rs1 = uc_free(op->gids) ;
	    if (rs >= 0) rs = rs1 ;
	    op->gids = NULL ;
	}
	if (rs >= 0) rs = ids_load(op) ;
	return rs ;
}
/* end subroutine (ids_refresh) */


/* copy constructor */
int ids_copy(IDS *op,IDS *otherp)
{
	int		rs ;
	op->uid = otherp->uid ;
	op->euid = otherp->euid ;
	op->gid = otherp->gid ;
	op->egid = otherp->egid ;
	if ((rs = ids_ngids(otherp)) >= 0) {
	    const int	n = rs ;
	    int		size = 0 ;
	    void	*p ;
	    size += ((n+1)*sizeof(gid_t)) ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        int	i ;
		op->gids = p ;
		for (i = 0 ; otherp->gids[i] ; i += 1) {
		    op->gids[i] = otherp->gids[i] ;
		} /* end for */
		op->gids[i] = -1 ;
	    } /* end if (m-a) */
	} /* end if (ids_ngids) */
	return rs ;
}
/* end subroutine (ids_refresh) */


/* private subroutines */


static int ids_ngids(IDS *op)
{
	int		i = 0 ;
	if (op->gids != NULL) {
	    for (i = 0 ; op->gids[i] >= 0 ; i += 1) ;
	}
	return i ;
}
/* end subroutine (ids_ngids) */


