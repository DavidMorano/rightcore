/* event */

/* a citation event */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1987-09-10, David A­D­ Morano
	This code module was originally written.

	= 1998-09-10, David A­D­ Morano
	This module was changed to serve in the REFERM program.

*/

/* Copyright © 1987,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code module contains a sinple object that is used to hold a
        citation event.


*******************************************************************************/


#define	EVENT_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"event.h"


/* local defines */

#define	EVENT_MAGIC	28

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif


/* external subroutines */


/* forward references */


/* external variables */


/* exported subroutines */


int event_start(op,offset,ckp)
EVENT		*op ;
const char	*ckp ;
{


	if (op == NULL)
		return SR_FAULT ;

	memset(op,0,sizeof(EVENT)) ;

	op->offset = offset ;
	op->citekey = ckp ;
	return SR_OK ;
}
/* end subroutine (event_start) */


int event_finish(op)
EVENT		*op ;
{


	if (op == NULL)
		return SR_FAULT ;

	op->offset = 0 ;
	op->citekey = NULL ;
	return SR_OK ;
}
/* end subroutine (event_finish) */


int event_offset(op,offp)
EVENT		*op ;
offset_t		*offp ;
{


	if (op == NULL)
		return SR_FAULT ;

	if (offp == NULL)
		return SR_FAULT ;

	*offp = op->offset ;
	return SR_OK ;
}
/* end subroutine (event_offset) */


/* get the entry under the current cursor */
int event_citekey(op,rpp)
EVENT		*op ;
const char	**rpp ;
{
	int	rs ;


	if (op == NULL)
		return SR_FAULT ;

	if (rpp == NULL)
		return SR_FAULT ;

	*rpp = op->citekey ;
	return rs ;
}
/* end subroutine (event_citekey) */



