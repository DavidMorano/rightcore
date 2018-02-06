/* useraccdb */

/* user-access (user-access-logging) data-base management */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	USERACCDB_INCLUDE
#define	USERACCDB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>		/* for 'uino_t' */
#include	<filebuf.h>
#include	<dater.h>
#include	<localmisc.h>


#define	USERACCDB	struct useraccdb_head
#define	USERACCDB_CUR	struct useraccdb_c
#define	USERACCDB_ENT	struct useraccdb_e
#define	USERACCDB_FL	struct useraccdb_flags


struct useraccdb_e {
	const char	*user ;
	const char	*name ;
	time_t		atime ;
	uint		count ;
} ;

struct useraccdb_c {
	FILEBUF		b ;
	offset_t	eo ;		/* enumeration offset */
} ;

struct useraccdb_flags {
	uint		locked:1 ;
	uint		dater:1 ;
} ;

struct useraccdb_head {
	uint		magic ;
	USERACCDB_FL	f ;
	DATER		dm ;		/* dater-manager */
	const char	*fname ;
	offset_t	eo ;
	time_t		ti_check ;
	time_t		ti_mod ;
	uino_t		ino ;
	dev_t		dev ;
	int		fd ;
} ;


#if	(! defined(USERACCDB_MASTER)) || (USERACCDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int useraccdb_open(USERACCDB *,const char *,const char *) ;
extern int useraccdb_find(USERACCDB *,USERACCDB_ENT *,char *,int,cchar *) ;
extern int useraccdb_update(USERACCDB *,const char *,const char *) ;
extern int useraccdb_curbegin(USERACCDB *,USERACCDB_CUR *) ;
extern int useraccdb_curend(USERACCDB *,USERACCDB_CUR *) ;
extern int useraccdb_enum(USERACCDB *,USERACCDB_CUR *,
		USERACCDB_ENT *,char *,int) ;
extern int useraccdb_check(USERACCDB *,time_t) ;
extern int useraccdb_close(USERACCDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* USERACCDB_MASTER */

#endif /* USERACCDB_INCLUDE */


