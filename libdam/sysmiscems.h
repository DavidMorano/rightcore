/* sysmiscems */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSMISCEMS_INCLUDE
#define	SYSMISCEMS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	SYSMISCEMS	struct sysmiscems_head

#define	SYSMISCEMS_OBJ	struct sysmiscems_obj
#define	SYSMISCEMS_DATA	struct sysmiscems_d


struct sysmiscems_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct sysmiscems_d {
	uint		intstale ;		/* "stale" interval */
	uint		utime ;
	uint		btime ;
	uint		ncpu ;
	uint		nproc ;
	uint		la[3] ;
} ;

struct sysmiscems_flags {
	uint		shm:1 ;			/* initialized */
} ;

struct sysmiscems_head {
	uint		magic ;
	const char	*pr ;
	const char	*prbuf ;
	const char	*shmname ;
	char		*mapdata ;
	uint		*shmtable ;
	struct sysmiscems_flags	f ;
	time_t		daytime ;
	time_t		ti_shm ;		/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* last check of file */
	size_t		mapsize ;
	int		nodenamelen ;
	int		pagesize ;
	int		shmsize ;
} ;


#if	(! defined(SYSMISCEMS_MASTER)) || (SYSMISCEMS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sysmiscems_open(SYSMISCEMS *,const char *) ;
extern int	sysmiscems_get(SYSMISCEMS *,time_t,int,SYSMISCEMS_DATA *) ;
extern int	sysmiscems_close(SYSMISCEMS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSMISCEMS_MASTER */

#endif /* SYSMISCEMS_INCLUDE */


