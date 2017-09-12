/* bvi */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BVI_INCLUDE
#define	BVI_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>
#include	"bvihdr.h"		/* this is the hash-file-header */


#define	BVI_MAGIC	0x88773421
#define	BVI_SUF		"bvi"		/* variable-index */

#define	BVI		struct bvi_head

#define	BVI_OBJ		struct bvi_obj
#define	BVI_QUERY	struct bvi_q
#define	BVI_CUR		struct bvi_c
#define	BVI_VERSE	struct bvi_v
#define	BVI_LINE	struct bvi_l
#define	BVI_INFO	struct bvi_i
#define	BVI_FMI		struct bvi_fmi


/* this is the shared-object description */
struct bvi_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct bvi_i {
	time_t		ctime ;
	time_t		mtime ;
	uint		maxbook ;
	uint		maxchapter ;
	uint		count ;
	uint		nzverses ;
} ;

struct bvi_q {
	uchar		b, c, v ;
} ;

struct bvi_l {
	uint		loff ;
	uint		llen ;
} ;

struct bvi_v {
	struct bvi_l	*lines ;
	uint		voff ;
	uint		vlen ;
	uchar		nlines, b, c, v ;
} ;

struct bvi_c {
	int		i ;
} ;

struct bvi_fmi {
	char		*mapdata ;	/* file map */
	time_t		ti_mod ;	/* time file modication */
	time_t		ti_map ;	/* time file map */
	size_t		mapsize ;
	uint		(*vt)[4] ;	/* mapped verses table */
	uint		(*lt)[2] ;	/* mapped lines table */
} ;

struct bvi_head {
	uint		magic ;
	const char 	*dbname ;
	const char	*fname ;
	BVI_FMI		fmi ;		/* file-map information */
	BVIHDR		fhi ;		/* file-header information */
	time_t		ti_lastcheck ;	/* time last check of file */
	int		ncursors ;
} ;


#if	(! defined(BVI_MASTER)) || (BVI_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bvi_open(BVI *,cchar *) ;
extern int	bvi_count(BVI *) ;
extern int	bvi_info(BVI *,BVI_INFO *) ;
extern int	bvi_read(BVI *,BVI_VERSE *,char *,int,BVI_QUERY *) ;
extern int	bvi_get(BVI *,BVI_QUERY *,BVI_VERSE *,char *,int) ;
extern int	bvi_curbegin(BVI *,BVI_CUR *) ;
extern int	bvi_enum(BVI *,BVI_CUR *,BVI_VERSE *,char *,int) ;
extern int	bvi_curend(BVI *,BVI_CUR *) ;
extern int	bvi_audit(BVI *) ;
extern int	bvi_chapters(BVI *,int,uchar *,int) ;
extern int	bvi_close(BVI *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BVI_MASTER */

#endif /* BVI_INCLUDE */


