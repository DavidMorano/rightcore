/* sysmiscs */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSMISCS_INCLUDE
#define	SYSMISCS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	SYSMISCS	struct sysmiscs_head
#define	SYSMISCS_OBJ	struct sysmiscs_obj
#define	SYSMISCS_DATA	struct sysmiscs_d
#define	SYSMISCS_FL	struct sysmiscs_flags

#define	SYSMISCS_MAGIC	0x66342125
#define	SYSMISCS_PROG	"msu"


struct sysmiscs_d {
	uint		utime ;
	uint		btime ;
	uint		ncpu ;
	uint		nproc ;
} ;

struct sysmiscs_flags {
	uint		msinit:1 ;		/* MS is initialized */
	uint		msopen:1 ;
	uint		msold:1 ;
} ;

struct sysmiscs_head {
	uint		magic ;
	const char	*pr ;
	const char	*nodename ;
	const char 	*dbname ;		/* DB-name */
	const char 	*dbfname ;		/* DB file-name */
	SYSMISCS_FL	f ;
	MSFILE		ms ;
	time_t		ti_db ;			/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* last check of file */
	int		nodenamelen ;
} ;


#if	(! defined(SYSMISCS_MASTER)) || (SYSMISCS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sysmiscs_open(SYSMISCS *,const char *) ;
extern int	sysmiscs_get(SYSMISCS *,time_t,SYSMISCS_DATA *) ;
extern int	sysmiscs_close(SYSMISCS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSMISCS_MASTER */

#endif /* SYSMISCS_INCLUDE */


