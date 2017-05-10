/* calcite */

/* calendar citation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano

	This object module was originally written.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This small object just holds a citation for a calendar entry.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"calcite.h"


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

/* local structures */


/* forward references */


/* exported variables */


/* local variables */


/* exported subroutines */


int calcite_load(CALCITE *ep,int y,int m, int d)
{

	if (ep == NULL) return SR_FAULT ;

	memset(ep,0,sizeof(CALCITE)) ;
	ep->y = (ushort) y ;
	ep->m = (uchar) m ;
	ep->d = (uchar) d ;
	return SR_OK ;
}
/* end subroutine (calcite_load) */


