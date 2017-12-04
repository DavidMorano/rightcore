/* procse */

/* expanded server entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	PROCSE_INCLUDE
#define	PROCSE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<varsub.h>
#include	<expcook.h>


#define	PROCSE		struct procse_head
#define	PROCSE_ARGS	struct procse_a


struct procse_a {
	const char	*passfile ;		/* pass-file */
	const char	*sharedobj ;		/* shared-object path */
	const char	*program ;		/* server program path */
	const char	*srvargs ;		/* server program arguments */
	const char	*username ;
	const char	*groupname ;
	const char	*options ;
	const char	*access ;
	const char	*failcont ;
} ;

struct procse_head {
	const char	**envv ;
	varsub		*vsp ;
	PROCSE_ARGS	a, *ap ;
} ;


#if	(! defined(PROCSE_MASTER)) || (PROCSE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int procse_start(PROCSE *,cchar **,varsub *,PROCSE_ARGS *) ;
extern int procse_process(PROCSE *,EXPCOOK *) ;
extern int procse_finish(PROCSE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROCSE_MASTER */

#endif /* PROCSE_INCLUDE */


