/* uc_gethostid */

/* interface component for UNIX® library-3c */
/* get the 'hostid' from the current machine */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	- 1998-06-16, David Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine gets the 'hostid' (supposedly something unique
	and somewhat standardized) from the current machine.

	NOTES:

	It is strange how this subroutine is a library subroutine and
	it returns a straight binary value of the host ID while the only
	kernel interface system call available for the similar function
	returns the host ID already formatted into a string of decimal
	characters!


*******************************************************************************/


#define	LIBUC_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* time outs */

#define	TO_DQUOT	(5 * 60)
#define	TO_NOSPC	(5 * 60)


/* exported subroutines */


int uc_gethostid(rp)
uint	*rp ;
{
	long	result ;


	if (rp == NULL)
	    return SR_FAULT ;

again:
	result = gethostid() ;

	if (rp != NULL)
	    *rp = (uint) result ;

	return SR_OK ;
}
/* end subroutine (uc_gethostid) */



