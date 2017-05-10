/* PROGENTRY */

/* program entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	PROGENTRY_INCLUDE
#define	PROGENTRY_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<varsub.h>
#include	<svcfile.h>


/* local object defines */

#define	PROGENTRY		struct progentry_head
#define	PROGENTRY_ARGS		struct progentry_a

#define	PROGENTRY_TMPDIR	"/tmp"
#define	PROGENTRY_IDLEN		14


struct progentry_a {
	const char	*version ;	/* %V */
	const char	*programroot ;	/* %R */
	const char	*nodename ;	/* %N */
	const char	*domainname ;	/* %D */
	const char	*hostname ;	/* %H */
	const char	*username ;	/* %U */
	const char	*groupname ;	/* %G */
	const char	*service ;	/* %s service name */
	const char	*interval ;	/* %i interval (decimal secs) */
	const char	*logid ;	/* ID for logging */
	const char	*tmpdname ;
	time_t		daytime ;	/* time of day (UNIX) */
} ;

struct progentry_flags {
	uint		srvargs:1 ;		/* initialized? */
} ;

struct progentry_head {
	unsigned long	magic ;
	struct progentry_flags	f ;
	vecstr	srvargs ;		/* server program arguments */
	varsub	*ssp ;			/* string substitutions */
	char	*program ;		/* server program path */
	char	*username ;
	char	*groupname ;
	char	*options ;
	char	*access ;		/* access hosts or groups */
	time_t	atime ;			/* job arrival time */
	pid_t	pid ;			/* run flag */
	int	interval ;		/* interval (seconds) */
	char	name[MAXNAMELEN + 1] ;	/* service name */
	char	jobid[PROGENTRY_IDLEN + 1] ;
	char	ofname[MAXPATHLEN + 1] ;
	char	efname[MAXPATHLEN + 1] ;
} ;


typedef struct progentry_head		progentry ;
typedef struct progentry_a		progentry_args ;


#ifndef	PROGENTRY_MASTER

#ifdef	__cplusplus
extern "C" {
#endif

extern int progentry_init(PROGENTRY *,VARSUB *,
			SVCFILE_ENT *,PROGENTRY_ARGS *) ;
extern int progentry_expand(PROGENTRY *,SVCFILE_ENT *,PROGENTRY_ARGS *) ;
extern int progentry_getinterval(PROGENTRY *,int *) ;
extern int progentry_getaccess(PROGENTRY *,char **) ;
extern int progentry_free(PROGENTRY *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROGENTRY_MASTER */


#endif /* PROGENTRY_INCLUDE */



