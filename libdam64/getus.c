/* getus */

/* get user-shell entries */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines were was written to provide a more consistent
	interface to the system 'user-shells" database.

	These subroutines are multi-thread safe, because the underlying
	subroutines are themselves multi-thread safe.

	Although safe for multithreading, different threads using these
	subroutines at the same time will enumerate separate subsets of the
	accessed database.  This is the way the UNIX® gods seemed to want these
	sorts of subroutines to work (did they really have a choice given that
	these existed before multi-threading came into vogue?).


*******************************************************************************/


#define	GETUS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"getus.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getus_begin()
{
	return uc_setus() ;
}
/* end subroutine (getus_begin) */


int getus_end()
{
	return uc_endus() ;
}
/* end subroutine (getus_end) */


int getus_ent(char *rbuf,int rlen)
{
	return uc_getus(rbuf,rlen) ;
}
/* end subroutine (getus_ent) */


