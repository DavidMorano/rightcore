/* strlistmk */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	STRLISTMK_INCLUDE
#define	STRLISTMK_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#include	"strlistmks.h"


#define	STRLISTMK	struct strlistmk_head
#define	STRLISTMK_CALLS	struct strlistmk_calls
#define	STRLISTMK_MAGIC	0x99447256


struct strlistmk_calls {
	int	(*open)(void *,const char *,const char *,int,mode_t,int) ;
	int	(*chgrp)(void *,gid_t) ;
	int	(*add)(void *,const char *,int) ;
	int	(*abort)(void *) ;
	int	(*close)(void *) ;
} ;

struct strlistmk_head {
	uint		magic ;
	MODLOAD		loader ;
	STRLISTMK_CALLS call ;
	void		*sop ;		/* shared-object (SO) pointer */
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
} ;


#if	(! defined(STRLISTMK_MASTER)) || (STRLISTMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	strlistmk_open(STRLISTMK *,const char *,const char *,
			int,mode_t,int) ;
extern int	strlistmk_chgrp(STRLISTMK *,gid_t) ;
extern int	strlistmk_add(STRLISTMK *,const char *,int) ;
extern int	strlistmk_abort(STRLISTMK *) ;
extern int	strlistmk_close(STRLISTMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRLISTMK_MASTER */

#endif /* STRLISTMK_INCLUDE */


