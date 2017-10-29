/* spawnproc */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SPAWNPROC_INCLUDE
#define	SPAWNPROC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* object */

#define	SPAWNPROC		struct spawnproc

/* options */

#define	SPAWNPROC_OIGNINTR	(1<<0)		/* ignore interrupts */
#define	SPAWNPROC_OSETSID	(1<<1)		/* set-session-id */
#define	SPAWNPROC_OSETPGRP	(1<<2)		/* set-proces-group ID */
#define	SPAWNPROC_OSETCTTY	(1<<3)		/* set control-term PGID */
#define	SPAWNPROC_OSIGDEFS	(1<<4)		/* set default signals */

/* file descriptor dispositions are */

#define	SPAWNPROC_DINHERIT	0			/* inherit this FD */
#define	SPAWNPROC_DCLOSE	1			/* close this FD */
#define	SPAWNPROC_DCREATE	2			/* create this one */
#define	SPAWNPROC_DOPEN		SPAWNPROC_DCREATE	/* create this one */
#define	SPAWNPROC_DDUP		3			/* DUP this one */


/* notes on this structure */

/****

This structure is part of the user interface for this facility.
The 'disp' components should be set by the caller to specify the desired
disposition for each of the three FDs of the child program ('0', '1', and
'2').  The 'fd' component serves as an input to the subroutine when the
corresponding disposition is DUP.  The 'fd' component is an output from
the subroutine when the corresponding disposition is OPEN.

****/


struct spawnproc {
	pid_t	pgrp ;
	int	opts ;
	int	disp[3] ;
	int	fd[3] ;
	int	fd_ctty ;
	int	nice ;
} ;


#if	(! defined(SPAWNPROC_MASTER)) || (SPAWNPROC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int spawnproc(struct spawnproc *,const char *,
		const char **,const char **) ;

#ifdef	__cplusplus
}
#endif

#endif /* SPAWNPROC_MASTER */

#endif /* SPAWNPROC_INCLUDE */


