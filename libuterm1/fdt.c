/* fdt */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"ucb.h"


/* local variables */

static FDT	fdt ;


/* forward references */

static int	fdt_get(FDT *,int,UCB **) ;
static int	fdt_free(FDT *,int) ;


/* private subroutines */


static int fdt_alloc(op,fd,rpp)
FDT	*op ;
int	fd ;
UCB	 **rpp ;
{
	FDT_ENT	*ep ;

	int	rs = SR_OK ;
	int	i ;
	int	size ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != FDT_MAGIC) {

		size = sizeof(FDT_ENT) ;
		rs = vecobj_start(&op->entries,size,10,0) ;

		if (rs < 0)
		goto ret0 ;

	} /* end if (initialization) */

/* do we already have it? */

	while (fdt_getentry(op,fd,&ep) >= 0)
			vecobj_del(&op->entries,i) ;

	for (i = 0 ; vecobj_get(&op->entries,i,&ep) >= 0 ; i += 1) {
		if (ep == NULL) continue ;

		if (ep->fd == fd) {


		}

	} /* end for */

/* allocate a new one */


ret0:
	return rs ;
}
/* end subroutine (fdt_alloc) */


static int fdt_getentry(op,fd,rpp)
FDT		*op ;
int		fd ;
FDT_ENT	**rpp ;
{
	FDT_ENT	*ep ;

	int	rs, i ;


	*rpp = NULL ;
	for (i = 0 ; (rs = vecobj_get(&op->entries,i,&ep)) >= 0 ; i += 1) {

		if (ep == NULL) continue ;

		if (ep->fd == fd)
			break ;

	} /* end for */

	if (rs >= 0)
		*rpp = ep ;

	return rs ;
}
/* end subroutine (fdt_getentry) */



