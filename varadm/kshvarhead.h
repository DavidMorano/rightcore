/* kshvar (INCLUDE) */

/* KSH variable framework */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	KSHVAR_INCLUDE
#define	KSHVAR_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecobj.h>


#define	KSHVAR		struct kshvar_head


struct kshvar_head {
	ulong		magic ;
	VECOBJ		vars ;
} ;

struct kshvar_c {
	int		i ;
} ;

struct kshvar_var {
	const char	*varname ;
	const char	*soname ;
	int		refcount ;
} ;


#if	(! defined(KSHVAR_MASTER)) || (KSHVAR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	kshvar_start(KSHVAR *) ;
extern int	kshvar_store(KSHVAR *,KSHVAR_VAR *) ;
extern int	kshvar_fetch(KSHVAR *,KSHVAR_VAR *) ;
extern int	kshvar_enum(KSHVAR *,KSHVAR_CUR *,KSHVAR_VAR *) ;
extern int	kshvar_finish(KSHVAR *) ;

#ifdef	__cplusplus
extern "C" {
#endif

#endif /* KSHVAR_MASTER */


#endif /* KSHVAR_INCLUDE */




