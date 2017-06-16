/* votdc */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VOTDC_INCLUDE
#define	VOTDC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<ptm.h>
#include	<shmalloc.h>
#include	<localmisc.h>

#include	"votdchdr.h"


#ifndef	SVCBUFLEN
#define	SVCBUFLEN		32
#endif

#define	VOTDC_MAGIC		0x43628190
#define	VOTDC_DEFLANG		"english"
#define	VOTDC_LANGLEN		SVCBUFLEN
#define	VOTDC_NTITLES		(66+1)
#define	VOTDC_NLANGS		4
#define	VOTDC_NBOOKS		4
#define	VOTDC_NVERSES		20
#define	VOTDC_BSTRSIZE		(VOTDC_NBOOKS*VOTDC_NTITLES*30)
#define	VOTDC_VSTRSIZE		(VOTDC_NVERSES*320)
#define	VOTDC			struct votdc_head
#define	VOTDC_FL		struct votdc_flags
#define	VOTDC_OBJ		struct votdc_obj
#define	VOTDC_BOOK		struct votdc_book
#define	VOTDC_VERSE		struct votdc_verse
#define	VOTDC_INFO		struct votdc_info
#define	VOTDC_CITE		struct votdc_cite
#define	VOTDC_Q			struct votdc_cite
#define	VOTDC_TC		struct votdc_titlecache
#define	VOTDC_VCUR		struct votdc_vcur


struct votdc_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct votdc_vcur {
	int		i ;
} ;

struct votdc_titlecache {
	const char	**titles ;
	const char	*a ;
	int		wmark ;
	int		amark ;
	char		lang[VOTDC_LANGLEN+1] ;
} ;

struct votdc_book {
	time_t		ctime ;
	time_t		atime ;
	int		wmark ;
	int		amark ;
	int		b[VOTDC_NTITLES+1] ;
	char		lang[VOTDC_LANGLEN+1] ;
} ;

struct votdc_verse {
	time_t		ctime ;
	time_t		atime ;
	int		mjd ;
	int		voff ;
	int		vlen ;
	int		wmark ;		/* write mark */
	int		amark ;		/* access mark */
	uchar		book, chapter, verse, lang ;
} ;

struct votdc_cite {
	uchar		b, c, v, l ;
} ;

struct votdc_info {
	time_t		wtime ;
	time_t		atime ;
	int		nbooks ;
	int		nverses ;
} ;

struct votdc_flags {
	uint		shm:1 ;
	uint		txt:1 ;
	uint		sorted:1 ;
} ;

struct votdc_head {
	uint		magic ;
	VOTDC_FL	f ;
	const char	*a ;		/* object string allocations */
	const char	*pr ;
	const char	*lang ;
	const char	*shmname ;
	caddr_t		mapdata ;	/* SHM data */
	PTM		*mp ;		/* pointer to SHM mutex */
	VOTDC_BOOK	*books ;	/* book-records */
	VOTDC_VERSE	*verses ;	/* verse-records */
	SHMALLOC	*ball ;		/* book-string allocator */
	SHMALLOC	*vall ;		/* verse-string allocator */
	char		*bstr ;		/* book string-table */
	char		*vstr ;		/* verse string-table */
	VOTDCHDR	hdr ;
	VOTDC_TC	tcs[VOTDC_NBOOKS] ;
	time_t		ti_map ;	/* map-time */
	time_t		ti_lastcheck ;
	size_t		mapsize ;	/* SHM map-size */
	int		pagesize ;
	int		shmsize ;
	int		nents ;	
	int		fd ;
} ;


#if	(! defined(VOTDC_MASTER)) || (VOTDC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	votdc_open(VOTDC *,cchar *,cchar *,int) ;
extern int	votdc_titlelang(VOTDC *,cchar *) ;
extern int	votdc_titleloads(VOTDC *,cchar *,cchar **) ;
extern int	votdc_titleget(VOTDC *,char *,int,int,int) ;
extern int	votdc_titlefetch(VOTDC *,char *,int,cchar *,int) ;
extern int	votdc_titlematch(VOTDC *,cchar *,cchar *,int) ;
extern int	votdc_vcurbegin(VOTDC *,VOTDC_VCUR *) ;
extern int	votdc_vcurenum(VOTDC *,VOTDC_VCUR *,VOTDC_CITE *,char *,int) ;
extern int	votdc_vcurend(VOTDC *,VOTDC_VCUR *) ;
extern int	votdc_verseload(VOTDC *,cchar *,VOTDC_CITE *,int,cchar *,int) ;
extern int	votdc_versefetch(VOTDC *,VOTDC_CITE *,char *,int,cchar *,int) ;
extern int	votdc_info(VOTDC *,VOTDC_INFO *) ;
extern int	votdc_close(VOTDC *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VOTDC_MASTER */

#endif /* VOTDC_INCLUDE */


