/* schedvar */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SCHEDVAR_INCLUDE
#define	SCHEDVAR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<vecstr.h>


#define	SCHEDVAR	VECSTR
#define	SCHEDVAR_CUR	struct schedvar_c


struct schedvar_c {
	int		i ;
} ;


#if	(! defined(SCHEDVAR_MASTER)) || (SCHEDVAR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int schedvar_start(SCHEDVAR *) ;
extern int schedvar_add(SCHEDVAR *,const char *,const char *,int) ;
extern int schedvar_curbegin(SCHEDVAR *,SCHEDVAR_CUR *) ;
extern int schedvar_enum(SCHEDVAR *,SCHEDVAR_CUR *,char *,int,char *,int) ;
extern int schedvar_curend(SCHEDVAR *,SCHEDVAR_CUR *) ;
extern int schedvar_findkey(SCHEDVAR *,const char *,const char **) ;
extern int schedvar_del(SCHEDVAR *,const char *) ;
extern int schedvar_expand(SCHEDVAR *,char *,int,const char *,int) ;
extern int schedvar_finish(SCHEDVAR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SCHEDVAR_MASTER */

#endif /* SCHEDVAR_INCLUDE */


