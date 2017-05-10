/* varmk */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VARMK_INCLUDE
#define	VARMK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<modload.h>
#include	<localmisc.h>

#include	"varmks.h"


#define	VARMK		struct varmk_head
#define	VARMK_CALLS	struct varmk_calls
#define	VARMK_MAGIC	0x99447246


struct varmk_calls {
	int	(*open)(void *,const char *,int,mode_t,int) ;
	int	(*chgrp)(void *,gid_t) ;
	int	(*addvar)(void *,const char *,const char *,int) ;
	int	(*abort)(void *) ;
	int	(*close)(void *) ;
} ;

struct varmk_head {
	uint		magic ;
	MODLOAD		loader ;
	VARMK_CALLS	call ;
	void		*sop ;		/* shared-object (SO) pointer */
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
} ;


#if	(! defined(VARMK_MASTER)) || (VARMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	varmk_open(VARMK *,cchar *,int,mode_t,int) ;
extern int	varmk_chgrp(VARMK *,gid_t) ;
extern int	varmk_addvar(VARMK *,cchar *,cchar *,int) ;
extern int	varmk_abort(VARMK *) ;
extern int	varmk_close(VARMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VARMK_MASTER */

#endif /* VARMK_INCLUDE */


