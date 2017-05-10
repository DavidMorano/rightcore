/* strfilemk */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	STRFILEMK_INCLUDE
#define	STRFILEMK_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#include	"strfilemks.h"


#define	STRFILEMK	struct strfilemk_head
#define	STRFILEMK_CALLS	struct strfilemk_calls
#define	STRFILEMK_MAGIC	0x99447256


struct strfilemk_calls {
	int	(*open)(void *,const char *,int,mode_t,int) ;
	int	(*chgrp)(void *,gid_t) ;
	int	(*addfile)(void *,const char *,int) ;
	int	(*abort)(void *) ;
	int	(*close)(void *) ;
} ;

struct strfilemk_head {
	uint		magic ;
	MODLOAD		loader ;
	STRFILEMK_CALLS call ;
	void		*sop ;		/* shared-object (SO) pointer */
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
} ;


#if	(! defined(STRFILEMK_MASTER)) || (STRFILEMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	strfilemk_open(STRFILEMK *,const char *,int,mode_t,int) ;
extern int	strfilemk_chgrp(STRFILEMK *,gid_t) ;
extern int	strfilemk_addfile(STRFILEMK *,const char *,int) ;
extern int	strfilemk_abort(STRFILEMK *) ;
extern int	strfilemk_close(STRFILEMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRFILEMK_MASTER */

#endif /* STRFILEMK_INCLUDE */


