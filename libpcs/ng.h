/* ng */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	NG_INCLUDE
#define	NG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/param.h>

#include	<vecitem.h>


struct newsgroup {
	const char	*name ;
	const char	*dir ;
	int		len ;
} ;


#define	NG		vecitem
#define	NG_ENT	struct newsgroup


#if	(! defined(NG_MASTER)) || (NG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ng_start(NG *) ;
extern int ng_addparse(NG *,const char *,int) ;
extern int ng_add(NG *,const char *,int,const char *) ;
extern int ng_copy(NG *,VECITEM *) ;
extern int ng_search(NG *,const char *,struct newsgroup **) ;
extern int ng_get(NG *,int,struct newsgroup **) ;
extern int ng_count(NG *) ;
extern int ng_finish(NG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* NG_MASTER */

#endif /* NG_INCLUDE */


