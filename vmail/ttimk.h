/* ttimk */

/* Termianl-Translate-Index file management */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	TTIMK_INCLUDE
#define	TTIMK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	TTIMK_MAGIC	0x88773422
#define	TTIMK		struct ttimk_head
#define	TTIMK_OBJ	struct ttimk_obj
#define	TTIMK_VERSE	struct ttimk_v
#define	TTIMK_LINE	struct ttimk_l
#define	TTIMK_INFO	struct ttimk_i


/* this is the shared-object description */
struct ttimk_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct ttimk_i {
	uint		maxbook ;
	uint		maxchapter ;
	uint		maxverse ;
	uint		nverses ;
	uint		nzverses ;
} ;

struct ttimk_l {
	uint		loff ;
	uint		llen ;
} ;

struct ttimk_v {
	struct ttimk_l	*lines ;
	uint		voff ;
	uint		vlen ;
	uchar		nlines, b, c, v ;
} ;

struct ttimk_flags {
	uint		notsorted:1 ;
	uint		creat:1 ;
	uint		excl:1 ;
	uint		none:1 ;
	uint		inprogress:1 ;
	uint		created:1 ;
} ;

struct ttimk_head {
	uint		magic ;
	const char 	*dbname ;
	const char	*idname ;
	char		*nfname ;
	struct ttimk_flags	f ;
	VECOBJ		verses ;
	VECOBJ		lines ;
	uint		pcitation ;
	uint		maxbook ;
	uint		maxchapter ;
	uint		maxverse ;
	uint		nverses ;
	uint		nzverses ;
	int		operms ;
	int		nfd ;
} ;


#if	(! defined(TTIMK_MASTER)) || (TTIMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ttimk_open(TTIMK *,const char *,int,int) ;
extern int	ttimk_add(TTIMK *,TTIMK_VERSE *) ;
extern int	ttimk_info(TTIMK *,TTIMK_INFO *) ;
extern int	ttimk_close(TTIMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TTIMK_MASTER */

#endif /* TTIMK_INCLUDE */



