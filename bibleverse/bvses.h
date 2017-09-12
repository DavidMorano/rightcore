/* bvses */

/* access manager interface to a Bible Verse Structure DB */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BVSES_INCLUDE
#define	BVSES_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>

#include	"bvshdr.h"		/* this has the file-header */


#define	BVSES_MAGIC	0x88773421
#define	BVSES_SUF	"bvs"		/* variable-index */

#define	BVSES		struct bvses_head
#define	BVSES_OBJ	struct bvses_obj
#define	BVSES_VERSE	struct bvses_v
#define	BVSES_INFO	struct bvses_i
#define	BVSES_FMI	struct bvses_fmi


struct bvses_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct bvses_i {
	time_t		ctime ;
	time_t		mtime ;
	uint		nzbooks ;		/* number of non-zero books */
	uint		nbooks ;
	uint		nchapters ;
	uint		nverses ;
	uint		nzverses ;
} ;

struct bvses_v {
	uchar		b, c, v ;
} ;

struct bvses_fmi {
	char		*mapdata ;	/* file map-data */
	time_t		ti_mod ;	/* time file modication */
	time_t		ti_map ;	/* time file map */
	size_t		mapsize ;	/* file map-size */
	ushort		(*bt)[4] ;	/* mapped book table */
	uchar		*ct ;		/* mapped chapter table */
} ;

struct bvses_head {
	uint		magic ;
	const char 	*pr ;
	const char 	*dbname ;
	const char	*fname ;
	BVSES_FMI	fmi ;		/* file-map information */
	BVSHDR		fhi ;		/* file-header information */
	time_t		ti_lastcheck ;	/* time last check of file */
	int		ncursors ;
} ;


#if	(! defined(BVSES_MASTER)) || (BVSES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bvses_open(BVSES *,const char *,const char *) ;
extern int	bvses_count(BVSES *) ;
extern int	bvses_info(BVSES *,BVSES_INFO *) ;
extern int	bvses_mkmodquery(BVSES *,BVSES_VERSE *,int) ;
extern int	bvses_audit(BVSES *) ;
extern int	bvses_close(BVSES *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BVSES_MASTER */

#endif /* BVSES_INCLUDE */


