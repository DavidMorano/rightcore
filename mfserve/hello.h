/* hello */

/* HELLO loadable service for MFSERVE */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	HELLO_INCLUDE
#define	HELLO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<pthread.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<vecpstr.h>
#include	<localmisc.h>

#include	"sreq.h"


/* object defines */

#define	HELLO			struct hello_head
#define	HELLO_FL		struct hello_flags
#define	HELLO_MAGIC		0x88773424


struct hello_flags {
	uint		args:1 ;
	uint		working:1 ;
} ;

struct hello_head {
	uint		magic ;
	PTM		m ;		/* mutex */
	PTC		c ;		/* condition variable */
	HELLO_FL	f ;
	SREQ		*jep ;
	VECPSTR		args ;
	volatile int	f_abort ;	/* command from parent thread */
	volatile int	f_exiting ;	/* thread is exiting */
	pid_t		pid ;
	pthread_t	tid ;
	int		ifd, ofd ;
	cchar		*pr ;
	cchar		**envv ;
} ;


#if	(! defined(HELLO_MASTER)) || (HELLO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int hello_start(HELLO *,cchar *,SREQ *,cchar **,cchar **) ;
extern int hello_check(HELLO *) ;
extern int hello_abort(HELLO *) ;
extern int hello_finish(HELLO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* HELLO_MASTER */

#endif /* HELLO_INCLUDE */


