/* bpi */

/* Bible Paragraph Index */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BPI_INCLUDE
#define	BPI_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>

#include	"bpihdr.h"		/* this is the hash-file-header */


#define	BPI_SUF		"bpi"		/* bible-paragraph-index */

#define	BPI		struct bpi_head

#define	BPI_OBJ		struct bpi_obj
#define	BPI_QUERY	struct bpi_q
#define	BPI_Q		struct bpi_q
#define	BPI_CUR		struct bpi_c
#define	BPI_VERSE	struct bpi_v
#define	BPI_INFO	struct bpi_i


/* this is the shared-object description */
struct bpi_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct bpi_i {
	time_t		ctime ;
	time_t		mtime ;
	uint		maxbook ;
	uint		maxchapter ;
	uint		count ;
	uint		nzverses ;
} ;

struct bpi_q {
	uchar		b, c, v ;
} ;

struct bpi_v {
	uchar		nlines, b, c, v ;
} ;

struct bpi_c {
	int		i ;
} ;

struct bpi_fmi {
	char		*mapdata ;	/* file map */
	uint		(*vt)[1] ;	/* mapped verses table */
	time_t		ti_mod ;	/* time file modication */
	time_t		ti_map ;	/* time file map */
	size_t		mapsize ;
} ;

struct bpi_head {
	uint		magic ;
	const char 	*dbname ;
	const char	*fname ;
	struct bpi_fmi	fmi ;		/* file-map information */
	BPIHDR		fhi ;		/* file-header information */
	time_t		ti_lastcheck ;	/* time last check of file */
	int		ncursors ;
} ;


#if	(! defined(BPI_MASTER)) || (BPI_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bpi_open(BPI *,const char *) ;
extern int	bpi_count(BPI *) ;
extern int	bpi_info(BPI *,BPI_INFO *) ;
extern int	bpi_get(BPI *,BPI_QUERY *) ;
extern int	bpi_curbegin(BPI *,BPI_CUR *) ;
extern int	bpi_enum(BPI *,BPI_CUR *,BPI_VERSE *) ;
extern int	bpi_curend(BPI *,BPI_CUR *) ;
extern int	bpi_audit(BPI *) ;
extern int	bpi_close(BPI *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BPI_MASTER */

#endif /* BPI_INCLUDE */


