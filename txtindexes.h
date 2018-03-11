/* txtindexes */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	TXTINDEXES_INCLUDE
#define	TXTINDEXES_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"txtindexhdr.h"		/* this is the hash-file-header */


#define	TXTINDEXES_MAGIC	0x88773421
#define	TXTINDEXES		struct txtindexes_head
#define	TXTINDEXES_OBJ		struct txtindexes_obj
#define	TXTINDEXES_CUR		struct txtindexes_c
#define	TXTINDEXES_TAG		struct txtindexes_tag
#define	TXTINDEXES_FI		struct txtindexes_fi
#define	TXTINDEXES_MI		struct txtindexes_mi
#define	TXTINDEXES_INFO		struct txtindexes_i


/* this is the shared-object description */
struct txtindexes_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct txtindexes_c {
	uint		*taglist ;
	uint		taglen ;
	int		i ;
} ;

struct txtindexes_tag {
	uint		recoff ;
	uint		reclen ;
	char		fname[MAXPATHLEN + 1] ;
} ;

struct txtindexes_fi {
	char		*mapdata ;
	time_t		ti_mod ;
	time_t		ti_map ;
	size_t		mapsize ;
} ;

struct txtindexes_mi {
	const char	*sdn ;
	const char	*sfn ;
	const char	*estab ;	/* eigen-string table */
	uint		*table ;	/* mapped hash table */
	uint		*lists ;	/* mapped lists */
	int		*ertab ;	/* eigen-record table */
	int		(*eitab)[3] ;	/* eigen-index table */
} ;

/* returned information */
struct txtindexes_i {
	time_t		ctime ;		/* index creation-time */
	time_t		mtime ;		/* index modification-time */
	uint		count ;		/* number of tags */
	uint		neigen ;
	uint		minwlen ;	/* minimum word length */
	uint		maxwlen ;	/* maximum word length */
	char		sdn[MAXPATHLEN + 1] ;
	char		sfn[MAXPATHLEN + 1] ;
} ;

struct txtindexes_head {
	uint		magic ;
	const char 	*dbname ;
	TXTINDEXES_FI	hf, tf ;
	TXTINDEXES_MI	mi ;
	TXTINDEXHDR	ifi ;		/* index-file (header) information */
	PTM		m ;
	time_t		ti_lastcheck ;	/* time last check of file */
	int		ncursors ;
} ;


#if	(! defined(TXTINDEXES_MASTER)) || (TXTINDEXES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	txtindexes_open(TXTINDEXES *,const char *) ;
extern int	txtindexes_count(TXTINDEXES *) ;
extern int	txtindexes_neigen(TXTINDEXES *) ;
extern int	txtindexes_info(TXTINDEXES *,TXTINDEXES_INFO *) ;
extern int	txtindexes_iseigen(TXTINDEXES *,const char *,int) ;
extern int	txtindexes_curbegin(TXTINDEXES *,TXTINDEXES_CUR *) ;
extern int	txtindexes_lookup(TXTINDEXES *,TXTINDEXES_CUR *,
			const char **) ;
extern int	txtindexes_read(TXTINDEXES *,TXTINDEXES_CUR *,
			TXTINDEXES_TAG *) ;
extern int	txtindexes_curend(TXTINDEXES *,TXTINDEXES_CUR *) ;
extern int	txtindexes_audit(TXTINDEXES *) ;
extern int	txtindexes_close(TXTINDEXES *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TXTINDEXES_MASTER */

#endif /* TXTINDEXES_INCLUDE */


