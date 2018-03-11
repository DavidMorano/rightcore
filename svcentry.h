/* SVCENTRY */

/* service entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SVCENTRY_INCLUDE
#define	SVCENTRY_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<vecstr.h>
#include	<varsub.h>
#include	<svcfile.h>


/* local object defines */

#define	SVCENTRY		struct svcentry_head
#define	SVCENTRY_ARGS		struct svcentry_a
#define	SVCENTRY_FL		struct svcentry_flags

#define	SVCENTRY_TMPDIR		"/tmp"
#define	SVCENTRY_IDLEN		14


struct svcentry_a {
	const char	*version ;	/* %V */
	const char	*programroot ;	/* %R */
	const char	*nodename ;	/* %N */
	const char	*domainname ;	/* %D */
	const char	*hostname ;	/* %H */
	const char	*username ;	/* %U */
	const char	*groupname ;	/* %G */
	const char	*service ;	/* %s service name */
	const char	*interval ;	/* %i interval (decimal secs) */
	const char	*jobid ;	/* ID for logging */
	const char	*tmpdname ;
	time_t		daytime ;	/* time of day (UNIX) */
} ;

struct svcentry_flags {
	uint		srvargs:1 ;	/* initialized? */
} ;

struct svcentry_head {
	uint		magic ;
	SVCENTRY_FL	f ;
	vecstr		srvargs ;	/* server program arguments */
	varsub		*ssp ;		/* string substitutions */
	const char	*program ;	/* server program path */
	const char	*username ;
	const char	*groupname ;
	const char	*options ;
	const char	*access ;	/* access hosts or groups */
	const char	*ofname ;
	const char	*efname ;
	time_t		atime ;		/* job arrival time */
	time_t		stime ;		/* job start time */
	pid_t		pid ;		/* run flag */
	int		interval ;	/* interval (seconds) */
	char		name[MAXNAMELEN + 1] ;	/* service name */
	char		jobid[SVCENTRY_IDLEN + 1] ;
} ;


typedef struct svcentry_head		srventry ;
typedef struct svcentry_a		svcentry_args ;


#if	(! defined(SVCENTRY_MASTER)) || (SVCENTRY_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int svcentry_start(SVCENTRY *,VARSUB *,SVCFILE_ENT *,SVCENTRY_ARGS *) ;
extern int svcentry_expand(SVCENTRY *,SVCFILE_ENT *,SVCENTRY_ARGS *) ;
extern int svcentry_getinterval(SVCENTRY *,int *) ;
extern int svcentry_arrival(SVCENTRY *,time_t *) ;
extern int svcentry_getaccess(SVCENTRY *,const char **) ;
extern int svcentry_getargs(SVCENTRY *,const char ***) ;
extern int svcentry_stime(SVCENTRY *,time_t) ;
extern int svcentry_finish(SVCENTRY *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SVCENTRY_MASTER */

#endif /* SVCENTRY_INCLUDE */


