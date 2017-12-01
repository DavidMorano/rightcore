/* pcsclient */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSCLIENT_INCLUDE
#define	PCSCLIENT_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	PCSCLIENT_MAGIC		0x58261227
#define	PCSCLIENT		struct pcsclient_head
#define	PCSCLIENT_OBJ		struct pcsclient_obj
#define	PCSCLIENT_DATA		struct pcsclient_d
#define	PCSCLIENT_FL		struct pcsclient_flags


struct pcsclient_obj {
	char		*name ;
	uint		objsize ;
} ;

struct pcsclient_d {
	uint		intstale ;
	uint		utime ;
	uint		btime ;
	uint		ncpu ;
	uint		nproc ;
	uint		la[3] ;
} ;

struct pcsclient_flags {
	uint		shm:1 ;			/* initialized */
} ;

struct pcsclient_head {
	ulong		magic ;
	cchar		*pr ;
	cchar		*shmname ;
	char		*mapdata ;
	uint		*shmtable ;
	PCSCLIENT_FL	f ;
	time_t		dt ;
	time_t		ti_shm ;		/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* last check of file */
	int		nodenamelen ;
	int		pagesize ;
	int		mapsize ;
	int		shmsize ;
} ;


#if	(! defined(PCSCLIENT_MASTER)) || (PCSCLIENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	pcsclient_open(PCSCLIENT *,const char *) ;
extern int	pcsclient_get(PCSCLIENT *,time_t,int,PCSCLIENT_DATA *) ;
extern int	pcsclient_close(PCSCLIENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSCLIENT_MASTER */

#endif /* PCSCLIENT_INCLUDE */


