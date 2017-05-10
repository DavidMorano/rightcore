/* msuclient */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MSUCLIENT_INCLUDE
#define	MSUCLIENT_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	MSUCLIENT_MAGIC		0x58261227
#define	MSUCLIENT		struct msuclient_head
#define	MSUCLIENT_OBJ		struct msuclient_obj
#define	MSUCLIENT_DATA		struct msuclient_d
#define	MSUCLIENT_FL		struct msuclient_flags


struct msuclient_obj {
	char		*name ;
	uint		objsize ;
} ;

struct msuclient_d {
	uint		intstale ;
	uint		utime ;
	uint		btime ;
	uint		ncpu ;
	uint		nproc ;
	uint		la[3] ;
} ;

struct msuclient_flags {
	uint		shm:1 ;			/* initialized */
} ;

struct msuclient_head {
	ulong		magic ;
	cchar		*pr ;
	cchar		*shmname ;
	char		*mapdata ;
	uint		*shmtable ;
	MSUCLIENT_FL	f ;
	time_t		dt ;
	time_t		ti_shm ;		/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* last check of file */
	int		nodenamelen ;
	int		pagesize ;
	int		mapsize ;
	int		shmsize ;
} ;


#if	(! defined(MSUCLIENT_MASTER)) || (MSUCLIENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	msuclient_open(MSUCLIENT *,const char *) ;
extern int	msuclient_get(MSUCLIENT *,time_t,int,MSUCLIENT_DATA *) ;
extern int	msuclient_close(MSUCLIENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MSUCLIENT_MASTER */

#endif /* MSUCLIENT_INCLUDE */


