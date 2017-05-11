/* bibleparas */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BIBLEPARAS_INCLUDE
#define	BIBLEPARAS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>

#include	"bpi.h"


#define	BIBLEPARAS_MAGIC	0x99447246
#define	BIBLEPARAS		struct bibleparas_head
#define	BIBLEPARAS_OBJ		struct bibleparas_obj
#define	BIBLEPARAS_FL		struct bibleparas_flags
#define	BIBLEPARAS_CITE		struct bibleparas_q
#define	BIBLEPARAS_Q		struct bibleparas_q
#define	BIBLEPARAS_CUR		struct bibleparas_c
#define	BIBLEPARAS_INFO		struct bibleparas_i

/* default DB name */
#define	BIBLEPARAS_DBNAME	"default"


/* this is the shared-object description */
struct bibleparas_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct bibleparas_i {
	time_t		dbtime ;		/* db-time */
	time_t		vitime ;		/* vi-time */
	uint		maxbook ;
	uint		maxchapter ;
	uint		nverses ;
	uint		nzverses ;
} ;

struct bibleparas_q {
	uchar		b, c, v ;
} ;

struct bibleparas_c {
	BPI_CUR		vicur ;
} ;

struct bibleparas_flags {
	uint		vind:1 ;		/* index is loaded */
} ;

struct bibleparas_head {
	uint		magic ;
	const char	*pr ;
	const char 	*dbname ;		/* DB-name */
	const char 	*dbfname ;		/* DB file-name */
	char		*mapdata ;		/* memory-map address */
	BIBLEPARAS_FL	f ;
	BPI		vind ;			/* verse-index */
	time_t		ti_db ;			/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* last check of file */
	time_t		ti_vind ;		/* verse-index */
	size_t		mapsize ;		/* map size */
	size_t		filesize ;		/* file size */
	int		nverses ;
	int		ncursors ;
} ;


#if	(! defined(BIBLEPARAS_MASTER)) || (BIBLEPARAS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bibleparas_open(BIBLEPARAS *,const char *,const char *) ;
extern int	bibleparas_count(BIBLEPARAS *) ;
extern int	bibleparas_ispara(BIBLEPARAS *,BIBLEPARAS_Q *) ;
extern int	bibleparas_curbegin(BIBLEPARAS *,BIBLEPARAS_CUR *) ;
extern int	bibleparas_enum(BIBLEPARAS *,BIBLEPARAS_CUR *,BIBLEPARAS_Q *) ;
extern int	bibleparas_curend(BIBLEPARAS *,BIBLEPARAS_CUR *) ;
extern int	bibleparas_audit(BIBLEPARAS *) ;
extern int	bibleparas_info(BIBLEPARAS *,BIBLEPARAS_INFO *) ;
extern int	bibleparas_close(BIBLEPARAS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLEPARAS_MASTER */

#endif /* BIBLEPARAS_INCLUDE */


