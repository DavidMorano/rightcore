/* bibleverses */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	BIBLEVERSES_INCLUDE
#define	BIBLEVERSES_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#include	"bvi.h"


#define	BIBLEVERSES_MAGIC	0x99447245
#define	BIBLEVERSES		struct bibleverses_head
#define	BIBLEVERSES_FL		struct bibleverses_flags
#define	BIBLEVERSES_OBJ		struct bibleverses_obj
#define	BIBLEVERSES_QUERY	struct bibleverses_q
#define	BIBLEVERSES_CITE	struct bibleverses_q
#define	BIBLEVERSES_Q		struct bibleverses_q
#define	BIBLEVERSES_CUR		struct bibleverses_c
#define	BIBLEVERSES_INFO	struct bibleverses_i
#define	BIBLEVERSES_I		struct bibleverses_i

#define	BIBLEVERSES_DBNAME	"av"


/* this is the shared-object description */
struct bibleverses_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct bibleverses_i {
	time_t		dbtime ;		/* db-time */
	time_t		vitime ;		/* vi-time */
	uint		maxbook ;
	uint		maxchapter ;
	uint		nverses ;
	uint		nzverses ;
} ;

struct bibleverses_q {
	uchar		b, c, v ;
} ;

struct bibleverses_c {
	BVI_CUR		vicur ;
} ;

struct bibleverses_flags {
	uint		vind:1 ;		/* index is loaded */
} ;

struct bibleverses_head {
	uint		magic ;
	const char	*pr ;
	const char 	*dbname ;		/* DB-name */
	const char 	*dbfname ;		/* DB file-name */
	char		*mapdata ;		/* memory-map address */
	BIBLEVERSES_FL	f ;
	BVI		vind ;			/* verse-index */
	time_t		ti_db ;			/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* last check of file */
	time_t		ti_vind ;		/* verse-index */
	size_t		mapsize ;		/* map size */
	size_t		filesize ;		/* file size */
	int		nverses ;
	int		ncursors ;
} ;


#if	(! defined(BIBLEVERSES_MASTER)) || (BIBLEVERSES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bibleverses_open(BIBLEVERSES *,cchar *,cchar *) ;
extern int	bibleverses_count(BIBLEVERSES *) ;
extern int	bibleverses_read(BIBLEVERSES *,char *,int,BIBLEVERSES_Q *) ;
extern int	bibleverses_get(BIBLEVERSES *,BIBLEVERSES_Q *,char *,int) ;
extern int	bibleverses_curbegin(BIBLEVERSES *,BIBLEVERSES_CUR *) ;
extern int	bibleverses_enum(BIBLEVERSES *,BIBLEVERSES_CUR *,
			BIBLEVERSES_QUERY *,char *,int) ;
extern int	bibleverses_curend(BIBLEVERSES *,BIBLEVERSES_CUR *) ;
extern int	bibleverses_audit(BIBLEVERSES *) ;
extern int	bibleverses_info(BIBLEVERSES *,BIBLEVERSES_INFO *) ;
extern int	bibleverses_chapters(BIBLEVERSES *,int,uchar *,int) ;
extern int	bibleverses_close(BIBLEVERSES *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLEVERSES_MASTER */

#endif /* BIBLEVERSES_INCLUDE */


