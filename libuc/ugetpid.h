/* ugetpid */

/* slightly tuned UNIX® look-alike for |getpid(2)| */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	UGETPID_INCLUDE
#define	UGETPID_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>


#if	(! defined(UGETPID_MASTER)) || (UGETPID_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ugetpid() ;
extern void	usetpid(pid_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* UGETPID_MASTER */

#endif /* UGETPID_INCLUDE */


