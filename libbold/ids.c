/* ids */

/* load up the process IDs */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable */


/* revision history:

	= 1998-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will load up the process user-group IDs.

	Synopsis:

	int ids_load(op)
	IDS	*op ;

	Arguments:

	op		pointer to object

	Returns:

	<0	error
	>=0	length of found filepath


*******************************************************************************/


#define	IDS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ids.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int ids_load(op)
IDS	*op ;
{
	int	n ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(struct ids)) ;

	op->uid = getuid() ;

	op->euid = geteuid() ;

	op->gid = getgid() ;

	op->egid = getegid() ;

	n = getgroups(NGROUPS_MAX,op->egids) ;
	if (n < 0)
	    n = 0 ;

	op->egids[n] = -1 ;
	return SR_OK ;
}
/* end subroutine (ids_load) */


int ids_release(op)
IDS	*op ;
{

	if (op == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (ids_release) */



