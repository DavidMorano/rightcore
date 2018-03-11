/* bvimk */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BVIMK_INCLUDE
#define	BVIMK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	BVIMK_MAGIC	0x88773422
#define	BVIMK		struct bvimk_head
#define	BVIMK_OBJ	struct bvimk_obj
#define	BVIMK_VERSE	struct bvimk_v
#define	BVIMK_LINE	struct bvimk_l
#define	BVIMK_INFO	struct bvimk_i
#define	BVIMK_FL	struct bvimk_flags
#define	BVIMK_INTOPEN	(10*60)
#define	BVIMK_INTSTALE	(5*60)


/* this is the object description */
struct bvimk_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct bvimk_i {
	uint		maxbook ;
	uint		maxchapter ;
	uint		maxverse ;
	uint		nverses ;
	uint		nzverses ;
} ;

struct bvimk_l {
	uint		loff ;
	uint		llen ;
} ;

struct bvimk_v {
	BVIMK_LINE	*lines ;
	uint		voff ;
	uint		vlen ;
	uchar		nlines, b, c, v ;
} ;

struct bvimk_flags {
	uint		notsorted:1 ;
	uint		ofcreat:1 ;
	uint		ofexcl:1 ;
	uint		inprogress:1 ;
	uint		created:1 ;
	uint		abort:1 ;
} ;

struct bvimk_head {
	uint		magic ;
	cchar 		*dbname ;
	cchar		*idname ;
	char		*nidxfname ;
	BVIMK_FL	f ;
	VECOBJ		verses ;
	VECOBJ		lines ;
	mode_t		om ;
	uint		pcitation ;
	uint		maxbook ;
	uint		maxchapter ;
	uint		maxverse ;
	uint		nverses ;
	uint		nzverses ;
	int		nfd ;
} ;


#if	(! defined(BVIMK_MASTER)) || (BVIMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bvimk_open(BVIMK *,cchar *,int,mode_t) ;
extern int	bvimk_add(BVIMK *,BVIMK_VERSE *) ;
extern int	bvimk_abort(BVIMK *,int) ;
extern int	bvimk_info(BVIMK *,BVIMK_INFO *) ;
extern int	bvimk_close(BVIMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BVIMK_MASTER */

#endif /* BVIMK_INCLUDE */


