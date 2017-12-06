/* getngroups */

/* get the maximum number of supplemetary groups allowed per process */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* compile-time debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we get the maximum number of supplemtary groups (GIDs) allowed per
	process.  The number is cached on the first fetch and then accessed
	from the cache on subsequent requests.  Yes, we are multithread safe.
	This number (maximum groups) cannot (usually) change without an
	intervening reboot of the system (thus it being completely acceptable
	to cache forever).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	GETNGROUPS	struct getngroups


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* local structures */

struct getngroups {
	int		n ;
} ;


/* forward references */


/* local variables */

static GETNGROUPS	getngroups_data ; /* zero-initialized */


/* exported subroutines */


int getngroups()
{
	GETNGROUPS	*gnp = &getngroups_data ;
	int		rs ;
	if (gnp->n == 0) {
	    const int	cmd = _SC_NGROUPS_MAX ;
	    if ((rs = uc_sysconf(cmd,NULL)) >= 0) {
	        gnp->n = rs ;
	    }
	} else {
	    rs = gnp->n ;
	}
	return rs ;
}
/* end subroutine (getngroups) */


