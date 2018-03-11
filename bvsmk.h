/* bvsmk */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BVSMK_INCLUDE
#define	BVSMK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	BVSMK_MAGIC	0x88773423
#define	BVSMK		struct bvsmk_head
#define	BVSMK_FL	struct bvsmk_flags
#define	BVSMK_OBJ	struct bvsmk_obj
#define	BVSMK_INTOPEN	(10*60)
#define	BVSMK_INTSTALE	(5*60)


/* this is the shared-object description */
struct bvsmk_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct bvsmk_flags {
	uint		notsorted:1 ;
	uint		ofcreat:1 ;
	uint		ofexcl:1 ;
	uint		inprogress:1 ;
	uint		created :1 ;
	uint		abort:1 ;
} ;

struct bvsmk_head {
	uint		magic ;
	cchar		*a ;		/* memory-allocation (pr, db) */
	cchar		*pr ;
	cchar 		*db ;
	cchar		*idname ;
	char		*nidxfname ;
	BVSMK_FL	f ;
	VECOBJ		books ;
	mode_t		om ;
	int		nverses ;
	int		nzverses ;
	int		maxbook ;
	int		nfd ;
} ;


#if	(! defined(BVSMK_MASTER)) || (BVSMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bvsmk_open(BVSMK *,const char *,const char *,int,mode_t) ;
extern int	bvsmk_add(BVSMK *,int,uchar *,int) ;
extern int	bvsmk_abort(BVSMK *,int) ;
extern int	bvsmk_close(BVSMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BVSMK_MASTER */

#endif /* BVSMK_INCLUDE */


