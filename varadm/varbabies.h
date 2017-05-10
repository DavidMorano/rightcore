/* varbabies (INCLUDE) */

/* KSH variable BABIES */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	VARBABIES_INCLUDE
#define	VARBABIES_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecobj.h>


#define	VARBABIES	struct varbabies_head


struct varbabies_head {
	ulong		magic ;
	VECOBJ		vars ;
} ;

struct varbabies_c {
	int		i ;
} ;

struct varbabies_var {
	const char	*varname ;
	const char	*soname ;
	int		refcount ;
} ;


#if	(! defined(VARBABIES_MASTER)) || (VARBABIES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	varbabies_set(VARBABIES *) ;
extern int	varbabies_store(VARBABIES *,VARBABIES_VAR *) ;
extern int	varbabies_fetch(VARBABIES *,VARBABIES_VAR *) ;
extern int	varbabies_enum(VARBABIES *,VARBABIES_CUR *,VARBABIES_VAR *) ;
extern int	varbabies_finish(VARBABIES *) ;

#ifdef	__cplusplus
extern "C" {
#endif

#endif /* VARBABIES_MASTER */


#endif /* VARBABIES_INCLUDE */




