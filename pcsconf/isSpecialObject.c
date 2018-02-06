/* isSpecialObject */

/* determine if a shared-object handle is special or not */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if a library file is a special object or not.

	Synopsis:

	int isSpecialObject(void *sop)

	Arguments:

	sop		shared-object handle

	Returns:

	1		shared-object is special
	0		shared-object is regular


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<dlfcn.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const void	*objs[] = {
	RTLD_DEFAULT,
	RTLD_NEXT,
	RTLD_SELF,
	RTLD_PROBE,
	NULL
} ;


/* exported subroutines */


int isSpecialObject(void *sop)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; objs[i] != NULL ; i += 1) {
	    f = (sop == objs[i]) ;
	    if (f) break ;
	} /* end if */

	return f ;
}
/* end subroutine (isSpecialObject) */


