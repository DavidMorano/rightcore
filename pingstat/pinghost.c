/* pinghost */

/* a little object to hold ping-host names */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This object serves as an entry (for a collection) of
	a 'pinghost'.


*******************************************************************************/

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"pinghost.h"


/* local structures */


/* exported subroutines */


int pinghost_start(op,hp,hl,min,to)
PINGHOST	*op ;
const char	hp[] ;
int		hl ;
int		min ;
int		to ;
{
	int	rs ;

	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (hp == NULL) return SR_FAULT ;

	memset(op,0,sizeof(PINGHOST)) ;
	op->intminping = min ;
	op->to = to ;

	if ((rs = uc_mallocstrw(hp,hl,&cp)) >= 0) {	
	    op->name = cp ;
	}

	return rs ;
}
/* end subroutine (pinghost_start) */


int pinghost_finish(op)
PINGHOST	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->name != NULL) {
	    rs1 = uc_free(op->name) ;
	    if (rs >= 0) rs = rs1 ;
	    op->name = NULL ;
	}

	return rs ;
}
/* end subroutine (pinghost_finish) */



