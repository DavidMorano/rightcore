/* envhelp */

/* help w/ handling environment */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	ENVHELP_INCLUDE
#define	ENVHELP_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<vechand.h>
#include	<strpack.h>


#define	ENVHELP		struct envhelp_head


struct envhelp_head {
	vechand		env ;
	strpack		stores ;
} ;


#if	(! defined(ENVHELP_MASTER)) || (ENVHELP_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int envhelp_start(ENVHELP *,cchar **,cchar **) ;
extern int envhelp_present(ENVHELP *,cchar *,int,cchar **) ;
extern int envhelp_envset(ENVHELP *,cchar *,cchar *,int) ;
extern int envhelp_sort(ENVHELP *) ;
extern int envhelp_getvec(ENVHELP *,cchar ***) ;
extern int envhelp_finish(ENVHELP *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(ENVHELP_MASTER)) || (ENVHELP_MASTER == 0) */

#endif /* ENVHELP_INCLUDE */


