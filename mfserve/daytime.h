/* daytime */

/* DAYTIME loadable service for MFSERVE */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DAYTIME_INCLUDE
#define	DAYTIME_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<pthread.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<vecpstr.h>
#include	<localmisc.h>


/* object defines */

#define	DAYTIME			struct daytime_head
#define	DAYTIME_FL		struct daytime_flags
#define	DAYTIME_MAGIC		0x88773424


struct daytime_flags {
	uint		args:1 ;
	uint		working:1 ;
} ;

struct daytime_head {
	uint		magic ;
	PTM		m ;		/* mutex */
	PTC		c ;		/* condition variable */
	DAYTIME_FL	f ;
	VECPSTR		args ;
	volatile int	f_abort ;	/* command from parent thread */
	volatile int	f_exiting ;	/* thread is exiting */
	pid_t		pid ;
	pthread_t	tid ;
	int		ifd, ofd ;
	cchar		*pr ;
	cchar		**argv ;
	cchar		**envv ;
} ;


#if	(! defined(DAYTIME_MASTER)) || (DAYTIME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int daytime_start(DAYTIME *,cchar *,cchar **,cchar **,int,int) ;
extern int daytime_check(DAYTIME *) ;
extern int daytime_abort(DAYTIME *) ;
extern int daytime_finish(DAYTIME *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DAYTIME_MASTER */

#endif /* DAYTIME_INCLUDE */


