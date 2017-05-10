/* bpimk */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BPIMK_INCLUDE
#define	BPIMK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	BPIMK_MAGIC	0x88773423
#define	BPIMK		struct bpimk_head
#define	BPIMK_OBJ	struct bpimk_obj
#define	BPIMK_VERSE	struct bpimk_v
#define	BPIMK_INFO	struct bpimk_i
#define	BPIMK_FL	struct bpimk_flags
#define	BPIMK_INTOPEN	(10*60)
#define	BPIMK_INTSTALE	(5*60)


/* this is the shared-object description */
struct bpimk_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct bpimk_i {
	uint		maxbook ;
	uint		maxchapter ;
	uint		maxverse ;
	uint		nverses ;
	uint		nzverses ;
} ;

struct bpimk_v {
	uchar		nlines, b, c, v ;
} ;

struct bpimk_flags {
	uint		notsorted:1 ;
	uint		ofcreat:1 ;
	uint		ofexcl:1 ;
	uint		inprogress:1 ;
	uint		created:1 ;
	uint		abort:1 ;
} ;

struct bpimk_head {
	uint		magic ;
	const char 	*dbname ;
	const char	*idname ;
	char		*nidxfname ;
	BPIMK_FL	f ;
	VECOBJ		verses ;
	mode_t		om ;
	uint		pcitation ;
	uint		maxbook ;
	uint		maxchapter ;
	uint		maxverse ;
	uint		nverses ;
	uint		nzverses ;
	int		nfd ;
} ;


#if	(! defined(BPIMK_MASTER)) || (BPIMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bpimk_open(BPIMK *,const char *,int,mode_t) ;
extern int	bpimk_add(BPIMK *,BPIMK_VERSE *) ;
extern int	bpimk_abort(BPIMK *,int) ;
extern int	bpimk_info(BPIMK *,BPIMK_INFO *) ;
extern int	bpimk_close(BPIMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BPIMK_MASTER */

#endif /* BPIMK_INCLUDE */


