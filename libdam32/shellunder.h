/* shellunder */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SHELLUNDER_INCLUDE
#define	SHELLUNDER_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>


#define	SHELLUNDER	struct shellunder


struct shellunder {
	const char	*progename ;	/* child program exec-name */
	pid_t		pid ;		/* parent (shell) PID */
} ;


#if	(! defined(SHELLUNDER_MASTER)) || (SHELLUNDER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	shellunder(SHELLUNDER *,cchar *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SHELLUNDER_MASTER */

#endif /* SHELLUNDER_INCLUDE */


