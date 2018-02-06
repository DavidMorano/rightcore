/* textmkind */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	TEXTMKIND_INCLUDE
#define	TEXTMKIND_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<eigendb.h>
#include	<vecobj.h>
#include	<psem.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"txtindexmk.h"
#include	"rtags.h"


#define	TEXTMKIND		struct textmkind_head
#define	TEXTMKIND_CUR		struct textmkind_c
#define	TEXTMKIND_OBJ		struct textmkind_obj
#define	TEXTMKIND_TAG		struct textmkind_tag
#define	TEXTMKIND_INFO		struct textmkind_i

/* query options */

#define	TEXTMKIND_OPREFIX	0x01		/* prefix match */


struct textmkind_i {
	time_t		ctime ;		/* index creation-time */
	time_t		mtime ;		/* index modification-time */
	uint		count ;		/* number of tags */
	uint		neigen ;
	uint		minwlen ;	/* minimum word length */
	uint		maxwlen ;	/* maximum word length */
	char		sdn[MAXPATHLEN + 1] ;
	char		sfn[MAXPATHLEN + 1] ;
} ;

struct textmkind_tag {
	uint		recoff ;
	uint		reclen ;
	char		fname[MAXPATHLEN + 1] ;
} ;

struct textmkind_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct textmkind_c {
	uint		magic ;
	RTAGS		tags ;
	RTAGS_CUR	tcur ;
	int		ntags ;
} ;

struct textmkind_flags {
	uint		ind:1 ;			/* text-index */
	uint		edb:1 ;
	uint		edbinit:1 ;
	uint		prefix:1 ;		/* prefix key-matches */
	uint		disp:1 ;
} ;

struct textmkind_head {
	uint		magic ;
	const char	*pr ;
	const char	*dbname ;		/* DB database name */
	const char	*basedname ;		/* base-directory */
	const char	*sdn ;
	const char	*sfn ;
	void		*disp ;
	struct textmkind_flags	f ;
	EIGENDB		edb ;
	TXTINDEX	ind ;
	time_t		ti_lastcheck ;
	time_t		ti_tind ;		/* text-index */
	int		pagesize ;
	int		dbfsize ;		/* DB file-size */
	int		mapsize ;		/* historial (for mapping) */
	int		minwlen ;		/* minimum key-word length */
	int		ncursors ;
	uchar		wterms[32] ;
} ;


#if	(! defined(TEXTMKIND_MASTER)) || (TEXTMKIND_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int textmkind_open(TEXTMKIND *,cchar *,cchar *,cchar *) ;
extern int textmkind_count(TEXTMKIND *) ;
extern int textmkind_info(TEXTMKIND *,TEXTMKIND_INFO *) ;
extern int textmkind_add(TEXTMKIND *,const char *,int) ;
extern int textmkind_close(TEXTMKIND *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TEXTMKIND_MASTER */

#endif /* TEXTMKIND_INCLUDE */


