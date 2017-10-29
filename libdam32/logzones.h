/* logzones */

/* log timezone names */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LOGZONES_INCLUDE
#define	LOGZONES_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* object defines */

#define	LOGZONES		struct logzones_head
#define	LOGZONES_ENT		struct logzones_e
#define	LOGZONES_CUR		struct logzones_c
#define	LOGZONES_FL		struct logzones_flags

#define	LOGZONES_MAGIC		91824563
#define	LOGZONES_ENTLEN		48
#define	LOGZONES_NENTS		100
#define	LOGZONES_ZNAMESIZE	8
#define	LOGZONES_STAMPSIZE	23
#define	LOGZONES_NOZONEOFFSET	(13 * 60)


struct logzones_e {
	uint		count ;
	short		off ;
	short		znl ;
	char		znb[LOGZONES_ZNAMESIZE + 1] ;
	char		st[LOGZONES_STAMPSIZE + 1] ;
} ;

struct logzones_flags {
	uint		writable:1 ;
	uint		lockedread:1 ;
	uint		lockedwrite:1 ;
	uint		cursor:1 ;		/* cursor-locked ? */
	uint		cursorlockbroken:1 ;	/* cursor lock broken */
	uint		cursoracc:1 ;		/* accessed while cursored ? */
	uint		remote:1 ;		/* remote mounted file */
} ;

struct logzones_head {
	uint		magic ;
	const char	*fname ;
	char		*buf ;
	LOGZONES_FL	f ;
	time_t		opentime ;		/* file open time */
	time_t		accesstime ;		/* file access time */
	time_t		mtime ;			/* file modification time */
	mode_t		operms ;
	int		oflags ;
	int		pagesize ;
	int		filesize ;
	int		bufsize ;
	int		fd ;
} ;

struct logzones_c {
	int		i ;
} ;


#if	(! defined(LOGZONES_MASTER)) || (LOGZONES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int logzones_open(LOGZONES *,const char *,int,mode_t) ;
extern int logzones_curbegin(LOGZONES *,LOGZONES_CUR *) ;
extern int logzones_curend(LOGZONES *,LOGZONES_CUR *) ;
extern int logzones_enum(LOGZONES *,LOGZONES_CUR *,LOGZONES_ENT *) ;
extern int logzones_match(LOGZONES *,const char *,int,int,LOGZONES_ENT *) ;
extern int logzones_update(LOGZONES *,const char *,int,int,const char *) ;
extern int logzones_check(LOGZONES *,time_t) ;
extern int logzones_close(LOGZONES *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LOGZONES_MASTER */

#endif /* LOGZONES_INCLUDE */


