/* envmgr */


/* revision history:

	= 1998-01-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	ENVMGR_INCLUDE
#define	ENVMGR_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<localmisc.h>


#define	ENVMGR_MAGIC	0x58261222
#define	ENVMGR		struct envmgr_head


struct envmgr_head {
	VECSTR		envstrs ;
	VECHAND		envlist ;
} ;


#if	(! defined(ENVMGR_MASTER)) || (ENVMGR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	envmgr_start(ENVMGR *) ;
extern int	envmgr_set(ENVMGR *,cchar *,cchar *,int) ;
extern int	envmgr_getvec(ENVMGR *,cchar ***) ;
extern int	envmgr_finish(ENVMGR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* ENVMGR_MASTER */

#endif /* ENVMGR_INCLUDE */


