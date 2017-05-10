/* PROGENTRY */

/* program entry */


#ifndef	PROGENTRY_INCLUDE
#define	PROGENTRY_INCLUDE	1



#include	<sys/types.h>
#include	<time.h>

#include	<varsub.h>
#include	<srvtab.h>



/* local object defines */

#define	PROGENTRY		struct progentry_head
#define	PROGENTRY_ARGS		struct progentry_a


#define	PROGENTRY_TMPDIR	"/tmp"
#define	PROGENTRY_IDLEN		14




struct progentry_a {
	char	*version ;		/* %V */
	char	*programroot ;		/* %R */
	char	*nodename ;		/* %N */
	char	*domainname ;		/* %D */
	char	*hostname ;		/* %H */
	char	*username ;		/* %U */
	char	*groupname ;		/* %G */
	char	*service ;		/* %s service name */
	char	*interval ;		/* %i interval (decimal secs) */
	char	*logid ;		/* ID for logging */
	char	*tmpdir ;
	time_t	daytime ;		/* time of day (UNIX) */
} ;

struct progentry_flags {
	uint	srvargs : 1 ;		/* initialized ? */
} ;

struct progentry_head {
	unsigned long	magic ;
	struct progentry_flags	f ;
	varsub	*ssp ;			/* string substitutions */
	vecstr	srvargs ;		/* server program arguments */
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



typedef struct progentry_head		srventry ;
typedef struct progentry_a		progentry_args ;



#ifndef	PROGENTRY_MASTER

extern int progentry_init(PROGENTRY *,VARSUB *,
			SRVTAB_ENT *,PROGENTRY_ARGS *) ;
extern int progentry_expand(PROGENTRY *,SRVTAB_ENT *,PROGENTRY_ARGS *) ;
extern int progentry_getinterval(PROGENTRY *,int *) ;
extern int progentry_free(PROGENTRY *) ;

#endif /* PROGENTRY_MASTER */


#endif /* PROGENTRY_INCLUDE */



