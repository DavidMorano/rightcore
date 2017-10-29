/* INCLUDE stackaddr */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STACKADDR_INCLUDE
#define	STACKADDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


#define	STACKADDR			struct stackaddr_head


struct stackaddr_head {
	char		*dbuf ;
	const char	*lhp ;
	int		lhl ;
	int		dlen ;
	int		i ;
	int		ri ;
} ;


#if	(! defined(STACKADDR_MASTER)) || (STACKADDR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	stackaddr_start(STACKADDR *,char *,int) ;
extern int	stackaddr_finish(STACKADDR *) ;
extern int	stackaddr_add(STACKADDR *,cchar *,int,cchar *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* STACKADDR_MASTER */

#endif /* STACKADDR_INCLUDE */


