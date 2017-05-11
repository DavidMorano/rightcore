/* lastlog */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LASTLOGFILE_INCLUDE
#define	LASTLOGFILE_INCLUDE	1


#undef	LOCAL_DARWIN
#define	LOCAL_DARWIN	\
	(defined(OSNAME_Darwin) && (OSNAME_Darwin > 0))


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>		/* needed for 'lastlog.h' */

#if		LOCAL_DARWIN
#include	<utmp.h>
#else
#include	<lastlog.h>
#endif

#include	<localmisc.h>


/* object defines */

#define	LASTLOGFILE_MAGIC	1092387457
#define	LASTLOGFILE		struct lastlogfile_head
#define	LASTLOGFILE_CUR		struct lastlogfile_c
#define	LASTLOGFILE_ENT		struct lastlog

/* other defines */

#define	LASTLOGFILE_FILEPATH	"/var/adm/lastlog"
#define	LASTLOGFILE_LHOST	16
#define	LASTLOGFILE_LLINE	8


struct lastlogfile_c {
	int		i ;
} ;

struct lastlogfile_head {
	uint		magic ;
	const char	*fname ;	/* stored file name */
	offset_t	fsize ;		/* file size */
	time_t		otime ;		/* open time (for FD caching) */
	time_t		mtime ;		/* last modification time */
	int		pagesize ;
	int		oflags ;	/* open flags */
	int		fd ;		/* file descriptor */
} ;


#if	(! defined(LASTLOGFILE_MASTER)) || (LASTLOGFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int lastlogfile_open(LASTLOGFILE *,const char *,int) ;
extern int lastlogfile_readinfo(LASTLOGFILE *,uid_t,time_t *,char *,char *) ;
extern int lastlogfile_writeinfo(LASTLOGFILE *,uid_t,time_t,
		const char *,const char *) ;
extern int lastlogfile_readentry(LASTLOGFILE *,uid_t,LASTLOGFILE_ENT *) ;
extern int lastlogfile_writeentry(LASTLOGFILE *,uid_t,LASTLOGFILE_ENT *) ;
extern int lastlogfile_check(LASTLOGFILE *,time_t) ;
extern int lastlogfile_close(LASTLOGFILE *) ;
extern int lastlogfile_curbegin(LASTLOGFILE *,LASTLOGFILE_CUR *) ;
extern int lastlogfile_curend(LASTLOGFILE *,LASTLOGFILE_CUR *) ;
extern int lastlogfile_enuminfo(LASTLOGFILE *,LASTLOGFILE_CUR *,
		uid_t *,time_t *,char *,char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LASTLOGFILE_MASTER */

#endif /* LASTLOGFILE_INCLUDE */


