/* bibleqs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	BIBLEQS_INCLUDE
#define	BIBLEQS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<eigendb.h>
#include	<localmisc.h>

#include	"txtindex.h"


#define	BIBLEQS_MAGIC	0x99889298
#define	BIBLEQS		struct bibleqs_head
#define	BIBLEQS_CUR	struct bibleqs_c
#define	BIBLEQS_OBJ	struct bibleqs_obj
#define	BIBLEQS_QUERY	struct bibleqs_q
#define	BIBLEQS_Q	struct bibleqs_q
#define	BIBLEQS_CITE	struct bibleqs_q
#define	BIBLEQS_FL	struct bibleqs_flags

/* query options */

#define	BIBLEQS_OPREFIX	0x01		/* prefix match */


struct bibleqs_q {
	uchar		b, c, v ;
} ;

struct bibleqs_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct bibleqs_c {
	uint		*verses ;		/* file-offsets to tags */
	uint		nverses ;
	int		i ;
} ;

struct bibleqs_flags {
	uint		ind:1 ;			/* text-index (opened) */
	uint		edb:1 ;
	uint		edbinit:1 ;
	uint		prefix:1 ;		/* prefix key-matches */
} ;

struct bibleqs_head {
	uint		magic ;
	const char	*pr ;
	const char	*dbname ;		/* DB database name */
	const char	*dbfname ;		/* DB filename */
	char		*dbmdata ;		/* DB map-data */
	BIBLEQS_FL	f ;
	EIGENDB		edb ;
	TXTINDEX	ind ;
	time_t		ti_db ;			/* DB file modification */
	time_t		ti_map ;		/* map */
	time_t		ti_lastcheck ;
	time_t		ti_tind ;		/* text-index */
	size_t		dbmsize ;		/* DB map-size */
	int		minwlen ;		/* minimum key-word length */
	int		ncursors ;
	uchar		wterms[32] ;
} ;


#if	(! defined(BIBLEQS_MASTER)) || (BIBLEQS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int bibleqs_open(BIBLEQS *,const char *,const char *) ;
extern int bibleqs_count(BIBLEQS *) ;
extern int bibleqs_audit(BIBLEQS *) ;
extern int bibleqs_curbegin(BIBLEQS *,BIBLEQS_CUR *) ;
extern int bibleqs_lookup(BIBLEQS *,BIBLEQS_CUR *,int,const char **) ;
extern int bibleqs_read(BIBLEQS *,BIBLEQS_CUR *,BIBLEQS_Q *,char *,int) ;
extern int bibleqs_curend(BIBLEQS *,BIBLEQS_CUR *) ;
extern int bibleqs_close(BIBLEQS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLEQS_MASTER */

#endif /* BIBLEQS_INCLUDE */


