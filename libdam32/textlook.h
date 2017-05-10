/* textlook */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	TEXTLOOK_INCLUDE
#define	TEXTLOOK_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<eigendb.h>
#include	<vecobj.h>
#include	<psem.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"txtindex.h"
#include	"rtags.h"


#define	TEXTLOOK_MAGIC	0x99889298
#define	TEXTLOOK	struct textlook_head
#define	TEXTLOOK_CUR	struct textlook_c
#define	TEXTLOOK_OBJ	struct textlook_obj
#define	TEXTLOOK_TAG	struct textlook_tag
#define	TEXTLOOK_INFO	struct textlook_i
#define	TEXTLOOK_FL	struct textlook_flags

/* query options */

#define	TEXTLOOK_OPREFIX	0x01		/* prefix match */


struct textlook_i {
	time_t		ctime ;		/* index creation-time */
	time_t		mtime ;		/* index modification-time */
	uint		count ;		/* number of tags */
	uint		neigen ;
	uint		minwlen ;	/* minimum word length */
	uint		maxwlen ;	/* maximum word length */
	char		sdn[MAXPATHLEN + 1] ;
	char		sfn[MAXPATHLEN + 1] ;
} ;

struct textlook_tag {
	uint		recoff ;
	uint		reclen ;
	char		fname[MAXPATHLEN + 1] ;
} ;

struct textlook_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct textlook_c {
	uint		magic ;
	RTAGS		tags ;
	RTAGS_CUR	tcur ;
	int		ntags ;
} ;

struct textlook_flags {
	uint		ind:1 ;			/* text-index */
	uint		edb:1 ;
	uint		edbinit:1 ;
	uint		prefix:1 ;		/* prefix key-matches */
} ;

struct textlook_head {
	uint		magic ;
	const char	*pr ;
	const char	*dbname ;		/* DB database name */
	const char	*basedname ;		/* base-directory */
	const char	*sdn ;
	const char	*sfn ;
	void		*disp ;
	TEXTLOOK_FL	f ;
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


#if	(! defined(TEXTLOOK_MASTER)) || (TEXTLOOK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int textlook_open(TEXTLOOK *,cchar *,cchar *,cchar *) ;
extern int textlook_count(TEXTLOOK *) ;
extern int textlook_info(TEXTLOOK *,TEXTLOOK_INFO *) ;
extern int textlook_curbegin(TEXTLOOK *,TEXTLOOK_CUR *) ;
extern int textlook_lookup(TEXTLOOK *,TEXTLOOK_CUR *,int,const char **) ;
extern int textlook_read(TEXTLOOK *,TEXTLOOK_CUR *,TEXTLOOK_TAG *) ;
extern int textlook_curend(TEXTLOOK *,TEXTLOOK_CUR *) ;
extern int textlook_audit(TEXTLOOK *) ;
extern int textlook_close(TEXTLOOK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TEXTLOOK_MASTER */

#endif /* TEXTLOOK_INCLUDE */


