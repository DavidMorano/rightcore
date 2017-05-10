/* mkprogenv */

/* make new environment for a program */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MKPROGENV_INCLUDE
#define	MKPROGENV_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<vechand.h>
#include	<strpack.h>


#define	MKPROGENV	struct mkprogenv_head


struct mkprogenv_head {
	const char	*uh ;
	const char	**envv ;
	vechand		env ;
	strpack		stores ;
	char		un[USERNAMELEN+1] ;
} ;


#if	(! defined(MKPROGENV_MASTER)) || (MKPROGENV_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mkprogenv_start(MKPROGENV *,const char **) ;
extern int mkprogenv_envset(MKPROGENV *,const char *,const char *,int) ;
extern int mkprogenv_getvec(MKPROGENV *,const char ***) ;
extern int mkprogenv_finish(MKPROGENV *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MKPROGENV_MASTER */

#endif /* MKPROGENV_INCLUDE */


