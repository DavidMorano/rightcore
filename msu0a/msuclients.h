/* msuclients */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MSUCLIENTS_INCLUDE
#define	MSUCLIENTS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<sockaddress.h>
#include	<localmisc.h>


#define	MSUCLIENTS_MAGIC	0x58261221
#define	MSUCLIENTS		struct msuclients_head
#define	MSUCLIENTS_OBJ		struct msuclients_obj
#define	MSUCLIENTS_DATA		struct msuclients_d
#define	MSUCLIENTS_FL		struct msuclients_flags


struct msuclients_obj {
	char		*name ;
	uint		objsize ;
} ;

struct msuclients_d {
	uint		pid ;		/* server PID */
	uint		utime ;		/* last update time */
	uint		btime ;		/* SYSMISC boot-time */
	uint		ncpu ;		/* SYSMISC ncpu */
	uint		nproc ;		/* SYSMISC nproc */
	uint		la[3] ;		/* SYSMISC load-averages */
} ;

struct msuclients_flags {
	uint		srv : 1 ;	/* initialized */
} ;

struct msuclients_head {
	ulong		magic ;
	const char	*pr ;
	const char	*tmpourdname ;
	const char	*clientfname ;
	const char	*reqfname ;
	MSUCLIENTS_FL	f ;
	SOCKADDRESS	srv ;		/* server address */
	time_t		dt ;
	int		srvlen ;	/* server address length */
	int		fd ;
	int		to ;
} ;


#if	(! defined(MSUCLIENTS_MASTER)) || (MSUCLIENTS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	msuclients_open(MSUCLIENTS *,cchar *,cchar *,int) ;
extern int	msuclients_status(MSUCLIENTS *) ;
extern int	msuclients_get(MSUCLIENTS *,time_t,MSUCLIENTS_DATA *) ;
extern int	msuclients_close(MSUCLIENTS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MSUCLIENTS_MASTER */

#endif /* MSUCLIENTS_INCLUDE */


