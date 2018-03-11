/* hostsfile */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	HOSTSFILE_INCLUDE
#define	HOSTSFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vecelem.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* object defines */

#define	HOSTSFILE_MAGIC		0x31415926
#define	HOSTSFILE		struct hostsfile_head
#define	HOSTSFILE_FILE		struct hostsfile_file
#define	HOSTSFILE_ENT		struct hostsfile_e
#define	HOSTSFILE_ERR		struct hostsfile_errline
#define	HOSTSFILE_CUR		struct hostsfile_c


/* other public defines */

#define	HOSTSFILE_FILEMAGIC	"hostscache"
#define	HOSTSFILE_CACHEDIR	"/var/tmp/hostscache"
#define	HOSTSFILE_CACHENAMELEN	14


struct hostsfile_c {
	int		i, j ;
} ;

struct hostsfile_head {
	uint		magic ;
	char		*cdir ;			/* cache directory */
	vecelem		files ;			/* files */
	vecelem		hes ;			/* host entries */
	time_t		checktime ;
} ;

struct hostsfile_file {
	const char	*hostsfname ;		/* file path name */
	const char	*cachefname ;
	time_t		mtime ;
} ;

struct hostsfile_e {
	uint		fi ;			/* file index */
	uint		ai ;			/* access index to record */
} ;

struct hostsfile_errline {
	const char	*filename ;
	int		line ;
} ;


typedef struct hostsfile_head		hostsfile ;
typedef struct hostsfile_ae		hostsfile_ent ;
typedef struct hostsfile_c		hostsfile_cur ;
typedef struct hostsfile_errline	hostsfile_err ;


#if	(! defined(HOTSFILE_MASTER)) || (HOTSFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int hostsfile_start(HOSTSFILE *,char *,char *,VECELEM *) ;
extern int hostsfile_fileadd(HOSTSFILE *,char *,VECELEM *) ;
extern int hostsfile_finish(HOSTSFILE *) ;
extern int hostsfile_enum(HOSTSFILE *,HOSTSFILE_CUR *,HOSTSFILE_ENT **) ;
extern int hostsfile_check(HOSTSFILE *,VECELEM *) ;
extern int hostsfile_curbegin(HOSTSFILE *,HOSTSFILE_CUR *) ;
extern int hostsfile_curend(HOSTSFILE *,HOSTSFILE_CUR *) ;
extern int hostsfile_allowed(HOSTSFILE *,char *,char *,char *,char *) ;
extern int hostsfile_anyallowed(HOSTSFILE *,vecstr *,vecstr *,char *,char *) ;

#ifdef	COMMENT
extern int hostsfile_find(HOSTSFILE *,char *,HOSTSFILE_ENT **) ;
#endif

#ifdef	__cplusplus
}
#endif

#endif /* HOSTSFILE_MASTER */

#endif /* HOSTSFILE_INCLUDE */


