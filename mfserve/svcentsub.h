/* svcentsub */

/* expanded server entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SVCENTSUB_INCLUDE
#define	SVCENTSUB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<varsub.h>
#include	<expcook.h>


#define	SVCENTSUB		struct svcentsub_head
#define	SVCENTSUB_ARGS	struct svcentsub_a


struct svcentsub_a {
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

struct svcentsub_head {
	const char	**envv ;
	varsub		*vsp ;
	SVCENTSUB_ARGS	a, *ap ;
} ;


#if	(! defined(SVCENTSUB_MASTER)) || (SVCENTSUB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int svcentsub_start(SVCENTSUB *,cchar **,varsub *,SVCENTSUB_ARGS *) ;
extern int svcentsub_process(SVCENTSUB *,EXPCOOK *) ;
extern int svcentsub_finish(SVCENTSUB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SVCENTSUB_MASTER */

#endif /* SVCENTSUB_INCLUDE */


