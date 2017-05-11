/* strlist */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STRLIST_INCLUDE
#define	STRLIST_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>

#include	"strlisthdr.h"		/* this is the hash-file-header */


#define	STRLIST_MAGIC	0x88773421
#define	STRLIST		struct strlist_head
#define	STRLIST_INFO	struct strlist_i

#define	STRLIST_OBJ	struct strlist_obj
#define	STRLIST_CUR	struct strlist_c
#define	STRLIST_FM	struct strlist_fm
#define	STRLIST_MI	struct strlist_mi


struct strlist_i {
	time_t		wtime ;
	time_t		mtime ;
	uint		nstrlist ;
	uint		nskip ;
} ;

/* this is the shared-object description */
struct strlist_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct strlist_c {
	uint		chash ;		/* "check" hash for key */
	int		i ;
} ;

struct strlist_fm {
	char		*mdata ;	/* file map */
	time_t		ti_mod ;
	time_t		ti_map ;
	size_t		msize ;
} ;

struct strlist_mi {
	int		(*rt)[1] ;	/* mapped record table */
	int		(*it)[3] ;	/* mapped key-index table */
	char		*kst ;		/* mapped key-string table */
} ;

struct strlist_head {
	uint		magic ;
	const char 	*dbname ;
	STRLIST_FM	vf ;		/* file map */
	STRLIST_MI	mi ;		/* memory index */
	STRLISTHDR	hdr ;		/* file header */
	time_t		ti_lastcheck ;	/* time last check of file */
	int		ncursors ;
} ;


#if	(! defined(STRLIST_MASTER)) || (STRLIST_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	strlist_open(STRLIST *,const char *) ;
extern int	strlist_count(STRLIST *) ;
extern int	strlist_curbegin(STRLIST *,STRLIST_CUR *) ;
extern int	strlist_look(STRLIST *,STRLIST_CUR *,const char *,int) ;
extern int	strlist_enum(STRLIST *,STRLIST_CUR *,char *,int) ;
extern int	strlist_curend(STRLIST *,STRLIST_CUR *) ;
extern int	strlist_info(STRLIST *,STRLIST_INFO *) ;
extern int	strlist_audit(STRLIST *) ;
extern int	strlist_close(STRLIST *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRLIST_MASTER */

#endif /* STRLIST_INCLUDE */


