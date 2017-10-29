/* votds */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VOTDS_INCLUDE
#define	VOTDS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<ptm.h>
#include	<shmalloc.h>
#include	<localmisc.h>

#include	"votdshdr.h"


#ifndef	SVCBUFLEN
#define	SVCBUFLEN		32
#endif

#define	VOTDS_MAGIC		0x43628190
#define	VOTDS_DEFLANG		"english"
#define	VOTDS_LANGLEN		SVCBUFLEN
#define	VOTDS_NTITLES		(66+1)
#define	VOTDS_NLANGS		4
#define	VOTDS_NBOOKS		4
#define	VOTDS_NVERSES		20
#define	VOTDS_BSTRSIZE		(VOTDS_NBOOKS*VOTDS_NTITLES*30)
#define	VOTDS_VSTRSIZE		(VOTDS_NVERSES*320)
#define	VOTDS			struct votds_head
#define	VOTDS_FL		struct votds_flags
#define	VOTDS_OBJ		struct votds_obj
#define	VOTDS_LANG		struct votds_lang
#define	VOTDS_BOOK		struct votds_book
#define	VOTDS_VERSE		struct votds_verse
#define	VOTDS_INFO		struct votds_info
#define	VOTDS_CITE		struct votds_cite
#define	VOTDS_Q			struct votds_cite
#define	VOTDS_TC		struct votds_titlecache


struct votds_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct votds_titlecache {
	const char	**titles ;
	const char	*a ;
	int		wmark ;
	int		amark ;
	char		lang[VOTDS_LANGLEN+1] ;
} ;

struct votds_lang {
	char		lang[VOTDS_LANGLEN+1] ;
} ;

struct votds_book {
	time_t		ctime ;
	time_t		atime ;
	int		wmark ;
	int		amark ;
	int		b[VOTDS_NTITLES+1] ;
	char		lang[VOTDS_LANGLEN+1] ;
} ;

struct votds_verse {
	time_t		ctime ;
	time_t		atime ;
	int		mjd ;
	int		voff ;
	int		vlen ;
	int		wmark ;
	int		amark ;
	uchar		book, chapter, verse, lang ;
} ;

struct votds_cite {
	uchar		b, c, v, l ;
} ;

struct votds_info {
	time_t		wtime ;
	time_t		atime ;
	int		nlangs ;
	int		nbooks ;
	int		nverses ;
} ;

struct votds_flags {
	uint		shm:1 ;
	uint		txt:1 ;
	uint		sorted:1 ;
} ;

struct votds_head {
	uint		magic ;
	VOTDS_FL	f ;
	const char	*pr ;
	const char	*lang ;
	const char	*shmname ;
	caddr_t		mapdata ;	/* SHM data */
	PTM		*mp ;		/* pointer to SHM mutex */
	VOTDS_LANG	*langs ;	/* lang-records */
	VOTDS_BOOK	*books ;	/* book-records */
	VOTDS_VERSE	*verses ;	/* verse-records */
	SHMALLOC	*ball ;		/* book-string allocator */
	SHMALLOC	*vall ;		/* verse-string allocator */
	char		*bstr ;		/* book string-table */
	char		*vstr ;		/* verse string-table */
	VOTDSHDR	hdr ;
	VOTDS_TC	tcs[VOTDS_NBOOKS] ;
	time_t		ti_map ;	/* map-time */
	time_t		ti_lastcheck ;
	size_t		mapsize ;	/* SHM map-size */
	int		pagesize ;
	int		shmsize ;
	int		nents ;	
	int		fd ;
} ;


#if	(! defined(VOTDS_MASTER)) || (VOTDS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	votds_open(VOTDS *,const char *,const char *,int) ;
extern int	votds_titlelang(VOTDS *,const char *) ;
extern int	votds_titleloads(VOTDS *,const char *,const char **) ;
extern int	votds_titlefetch(VOTDS *,char *,int,const char *,int) ;
extern int	votds_titlematch(VOTDS *,const char *,const char *,int) ;
extern int	votds_verseload(VOTDS *,const char *,
			VOTDS_CITE *,int,const char *,int) ;
extern int	votds_versefetch(VOTDS *,VOTDS_CITE *,
			char *,int,const char *,int) ;
extern int	votds_info(VOTDS *,VOTDS_INFO *) ;
extern int	votds_close(VOTDS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VOTDS_MASTER */

#endif /* VOTDS_INCLUDE */


