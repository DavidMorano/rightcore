/* babycalcs */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BABYCALCS_INCLUDE
#define	BABYCALCS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<ptm.h>
#include	<localmisc.h>

#include	"babiesfu.h"


#define	BABYCALCS_DBNAME	"babies"
#define	BABYCALCS_MAGIC		0x43628199
#define	BABYCALCS		struct babycalcs_head
#define	BABYCALCS_OBJ		struct babycalcs_obj
#define	BABYCALCS_INFO		struct babycalcs_i
#define	BABYCALCS_FL		struct babycalcs_flags
#define	BABYCALCS_ENT		struct babycalcs_e


struct babycalcs_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct babycalcs_i {
	time_t		wtime ;
	time_t		atime ;
	uint		acount ;
} ;

struct babycalcs_e {
	time_t		date ;
	uint		count ;
} ;

struct babycalcs_flags {
	uint		shm:1 ;
	uint		txt:1 ;
	uint		sorted:1 ;
	uint		needinit:1 ;
} ;

struct babycalcs_head {
	uint		magic ;
	BABYCALCS_FL	f ;
	BABYCALCS_ENT	*table ;
	const char	*pr ;
	const char	*dbfname ;
	const char	*shmname ;
	caddr_t		mapdata ;	/* SHM data */
	PTM		*mp ;		/* pointer to SHM mutex */
	BABIESFU	hf ;
	time_t		ti_mdb ;	/* db-mtime */
	time_t		ti_map ;	/* map-time */
	time_t		ti_lastcheck ;
	size_t		mapsize ;	/* SHM map-size */
	int		pagesize ;
	int		dbsize ;
	int		shmsize ;
	int		nentries ;	
} ;


#if	(! defined(BABYCALCS_MASTER)) || (BABYCALCS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	babycalcs_open(BABYCALCS *,cchar *,cchar *) ;
extern int	babycalcs_check(BABYCALCS *,time_t) ;
extern int	babycalcs_lookup(BABYCALCS *,time_t,uint *) ;
extern int	babycalcs_info(BABYCALCS *,BABYCALCS_INFO *) ;
extern int	babycalcs_close(BABYCALCS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BABYCALCS_MASTER */

#endif /* BABYCALCS_INCLUDE */


