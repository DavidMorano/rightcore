/* lockfile(3dam) */

/* sister subroutine to |uc_lockf(3uc)| */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	LOCKFILE_INCLUDE
#define	LOCKFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


#if	(! defined(LOCKFILE_MASTER)) || (LOCKFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	lockfile(int,int,offset_t,offset_t,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* LOCKFILE_MASTER */

#endif /* LOCKFILE_INCLUDE */


