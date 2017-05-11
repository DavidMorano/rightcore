/* sysmiscers */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSMISCERS_INCLUDE
#define	SYSMISCERS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	SYSMISCERS	struct sysmiscers_head

#define	SYSMISCERS_OBJ	struct sysmiscers_obj
#define	SYSMISCERS_DATA	struct sysmiscers_d


struct sysmiscers_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct sysmiscers_d {
	uint		utime ;
	uint		btime ;
	uint		ncpu ;
	uint		nproc ;
} ;

struct sysmiscers_flags {
	uint		msinit:1 ;		/* MS is initialized */
	uint		msopen:1 ;
	uint		msold:1 ;
} ;

struct sysmiscers_head {
	uint		magic ;
	const char	*pr ;
	const char	*nodename ;
	const char 	*dbname ;		/* DB-name */
	const char 	*dbfname ;		/* DB file-name */
	struct sysmiscers_flags	f ;
	MSFILE		ms ;
	time_t		ti_db ;			/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* last check of file */
	int		nodenamelen ;
} ;


#if	(! defined(SYSMISCERS_MASTER)) || (SYSMISCERS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sysmiscers_open(SYSMISCERS *,const char *) ;
extern int	sysmiscers_get(SYSMISCERS *,time_t,SYSMISCERS_DATA *) ;
extern int	sysmiscers_close(SYSMISCERS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSMISCERS_MASTER */

#endif /* SYSMISCERS_INCLUDE */


