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
#include	<ptm.h>
#include	<ptc.h>
#include	<localmisc.h>


/* object defines */

#define	DAYTIME			struct daytime_head
#define	DAYTIME_FL		struct daytime_flags
#define	DAYTIME_OBJ		struct daytime_obj
#define	DAYTIME_MAGIC		0x88773424
#define	DAYTIME_DEFENTS		5


struct daytime_obj {
	cchar		*objname ;
	int		objsize ;
	int		end ;
} ;

struct daytime_flags {
	uint		dummy:1 ;
} ;

struct daytime_head {
	uint		magic ;
	PTM		m ;		/* mutex */
	PTC		c ;		/* condition variable */
	DAYTIME_FL	f ;
	volatile int	f_abort ;	/* command from parent thread */
	int		dummy ;
} ;


#if	(! defined(DAYTIME_MASTER)) || (DAYTIME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int daytime_start(DAYTIME *,cchar *,cchar *,cchar **,cchar **) ;
extern int daytime_abort(DAYTIME *) ;
extern int daytime_finish(DAYTIME *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DAYTIME_MASTER */

#endif /* DAYTIME_INCLUDE */


