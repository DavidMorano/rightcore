/* bvs */

/* Bible Verse Structure */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BVS_INCLUDE
#define	BVS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<modload.h>
#include	<localmisc.h>

#include	"bvses.h"


#define	BVS_MAGIC	0x97677246
#define	BVS		struct bvs_head
#define	BVS_DATA	struct bvs_d
#define	BVS_VERSE	struct bvs_v
#define	BVS_INFO	struct bvs_i
#define	BVS_CALLS	struct bvs_calls


struct bvs_v {
	uchar		b, c, v ;
} ;

struct bvs_i {
	time_t		ctime ;
	time_t		mtime ;
	uint		nzbooks ;		/* number of non-zero books */
	uint		nbooks ;
	uint		nchapters ;
	uint		nverses ;
	uint		nzverses ;
} ;

struct bvs_calls {
	int	(*open)(void *,const char *,const char *) ;
	int	(*count)(void *) ;
	int	(*info)(void *,BVSES_INFO *) ;
	int	(*mkmodquery)(void *,BVSES_VERSE *,int) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct bvs_head {
	uint		magic ;
	MODLOAD		loader ;
	BVS_CALLS	call ;
	void		*obj ;		/* object pointer */
} ;


#if	(! defined(BVS_MASTER)) || (BVS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bvs_open(BVS *,const char *,const char *) ;
extern int	bvs_count(BVS *) ;
extern int	bvs_info(BVS *,BVS_INFO *) ;
extern int	bvs_mkmodquery(BVS *,BVS_VERSE *,int) ;
extern int	bvs_audit(BVS *) ;
extern int	bvs_close(BVS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BVS_MASTER */

#endif /* BVS_INCLUDE */


