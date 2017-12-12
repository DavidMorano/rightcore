/* eaccess */

/* check file access for the current process by its effective UID */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1999-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Check access by effective UID.

	Synopsis:

	int eaccess(cchar *fname,int am)

	Arguments:

	fname		file-name to check
	am		access mode

	Returns:

	<0		error in dialing
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sperm(IDS *,USTAT *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int eaccess(cchar *fname,int am)
{
	return perm(fname,-1,-1,NULL,am) ;
}
/* end subroutine (eaccess) */


