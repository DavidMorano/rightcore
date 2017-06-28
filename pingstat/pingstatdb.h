/* pingstatdb */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PINGSTATDB_INCLUDE
#define	PINGSTATDB_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/timeb.h>
#include	<netdb.h>

#include	<bfile.h>
#include	<vecitem.h>
#include	<dater.h>
#include	<localmisc.h>


/* object defines */

#define	PINGSTATDB_MAGIC	0x31415926
#define	PINGSTATDB		struct pingstatdb_head
#define	PINGSTATDB_CUR		struct pingstatdb_c
#define	PINGSTATDB_ENT		struct pingstatdb_e
#define	PINGSTATDB_UP		struct pingstatdb_u
#define	PINGSTATDB_FL		struct pingstatdb_flags


struct pingstatdb_flags {
	uint		readlocked:1 ;		/* file is read locked */
	uint		writelocked:1 ;		/* file is write locked */
	uint		cached:1 ;		/* file is cached */
	uint		writable:1 ;		/* file is writable */
	uint		cursor:1 ;		/* cursor is out */
	uint		tzset:1 ;		/* was tzset() called */
} ;

struct pingstatdb_head {
	uint		magic ;
	const char	*fname ;
	bfile		pfile ;
	vecitem		entries ;
	struct timeb	now ;
	time_t		mtime ;
	PINGSTATDB_FL	f ;
	char		zname[DATER_ZNAMESIZE] ;
} ;

struct pingstatdb_c {
	int		i ;
} ;

struct pingstatdb_e {
	time_t		ti_change ;	/* last change */
	time_t		ti_ping ;	/* last ping */
	uint		count ;
	int		f_up ;		/* UP-DOWN status */
	char		hostname[MAXHOSTNAMELEN+1] ;
} ;

struct pingstatdb_u {
	uint		timestamp ;
	uint		timechange ;
	uint		count ;
} ;


#if	(! defined(PINGSTATDB_MASTER)) || (PINGSTATDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pingstatdb_open(PINGSTATDB *,const char *,mode_t,int) ;
extern int pingstatdb_close(PINGSTATDB *) ;
extern int pingstatdb_match(PINGSTATDB *,const char *,
		PINGSTATDB_ENT *) ;
extern int pingstatdb_curbegin(PINGSTATDB *,PINGSTATDB_CUR *) ;
extern int pingstatdb_curend(PINGSTATDB *,PINGSTATDB_CUR *) ;
extern int pingstatdb_enum(PINGSTATDB *,PINGSTATDB_CUR *,
		PINGSTATDB_ENT *) ;
extern int pingstatdb_update(PINGSTATDB *,const char *,int,time_t) ;
extern int pingstatdb_uptime(PINGSTATDB *,const char *,PINGSTATDB_UP *) ;
extern int pingstatdb_check(PINGSTATDB *,time_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* PINGSTATDB_MASTER */

#endif /* PINGSTATDB_INCLUDE */


