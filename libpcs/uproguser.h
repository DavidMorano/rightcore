/* uproguser */

/* UNIX® username and possibly user-home-directory */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	UPROGUSER_INCLUDE
#define	UPROGUSER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>


#if	(! defined(UPROGUSER_MASTER)) || (UPROGUSER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uproguser_init() ;
extern int uproguser_nameset(cchar *,int,uid_t,int) ;
extern int uproguser_nameget(char *,int,uid_t) ;
extern void uproguser_fini() ;

#ifdef	__cplusplus
}
#endif

#endif /* UPROGUSER_MASTER */

#endif /* UPROGUSER_INCLUDE */


