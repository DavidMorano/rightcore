/* ts (Time-Stamp) */

/* time-stamp file manager */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	TS_INCLUDE
#define	TS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<mapstrint.h>
#include	<localmisc.h>

#include	"tse.h"
#include	"ebuf.h"


/* object defines */

#define	TS_MAGIC		918245636
#define	TS			struct ts_head
#define	TS_CUR			struct ts_c
#define	TS_ENT			TSE_ALL

#define	TS_FILEMAGIC		"MS"
#define	TS_FILEMAGICSIZE	16
#define	TS_FILEMAGICLEN		sizeof(TS_FILEMAGIC)
#define	TS_FILEVERSION		0
#define	TS_ENDIAN		0
#define	TS_KEYNAMELEN		TSE_LKEYNAME
#define	TS_IDLEN		(16 + sizeof(uint))
#define	TS_HEADTABLEN		(3 * sizeof(uint))
#define	TS_TOPLEN		(TS_IDLEN + TS_HEADTABLEN)

/* entry flags */
#define	TS_FLA			0x01	/* flag-loadaverage */
#define	TS_FUSERS		0x02	/* flag-users */
#define	TS_FPMAVAIL		0x04	/* flag-percent_memory_available */


struct ts_h {
	uint		nentries ;
	uint		wtime ;
	uint		wcount ;
} ;

struct ts_flags {
	uint		fileinit:1 ;		/* file init'ed */
	uint		writable:1 ;
	uint		lockedread:1 ;
	uint		lockedwrite:1 ;
	uint		cursorlockbroken:1 ;	/* cursor lock broken */
	uint		cursoracc:1 ;		/* accessed while cursored? */
	uint		remote:1 ;		/* remote mounted file */
	uint		bufvalid:1 ;		/* buffer valid */
	uint		ebuf:1 ;		/* EBUF active */
} ;

struct ts_head {
	uint		magic ;
	const char	*fname ;
	struct ts_flags	f ;
	struct ts_h	h ;
	EBUF		ebm ;			/* entry-buffer-manager */
	MAPSTRINT	ni ;			/* nodename index */
	time_t		ti_open ;		/* file open time */
	time_t		ti_access ;		/* file access time */
	time_t		ti_mod ;		/* file modification time */
	mode_t		operm ;
	int		oflags ;
	int		pagesize ;
	int		filesize ;
	int		topsize ;
	int		fd ;
	int		ncursors ;
	int		fileversion, fileencoding, filetype ;
	char		topbuf[TS_TOPLEN + 1] ;	/* top-buffer */
} ;

struct ts_c {
	int		i ;
} ;


#if	(! defined(TS_MASTER)) || (TS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ts_open(TS *,const char *,int,mode_t) ;
extern int ts_curbegin(TS *,TS_CUR *) ;
extern int ts_curend(TS *,TS_CUR *) ;
extern int ts_enum(TS *,TS_CUR *,TS_ENT *) ;
extern int ts_match(TS *,time_t,const char *,int,TS_ENT *) ;
extern int ts_write(TS *,time_t,const char *,int,TS_ENT *) ;
extern int ts_update(TS *,time_t,TS_ENT *) ;
extern int ts_check(TS *,time_t) ;
extern int ts_count(TS *) ;
extern int ts_close(TS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TS_MASTER */

#endif /* TS_INCLUDE */


