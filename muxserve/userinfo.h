/* userinfo */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	USERINFO_INCLUDE
#define	USERINFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<vsystem.h>
#include	<netdb.h>


/* miscellaneous defines */


/* object defines */

#define	USERINFO		struct userinfo
#define	USERINFO_FL		struct userinfo_flags
#define	USERINFO_MAGIC		0x33216271
#define	USERINFO_LEN		((3 * 2048) + MAXHOSTNAMELEN)
#define	USERINFO_LOGIDLEN	15


struct userinfo_flags {
	unsigned int	sysv_rt:1 ;
	unsigned int	sysv_ct:1 ;
} ;

struct userinfo {
	uint		magic ;
	const char	*sysname ;	/* UNAME OS system-name */
	const char	*release ;	/* UNAME OS release */
	const char	*version ;	/* UNAME OS version */
	const char	*machine ;	/* UNAME machine */
	const char	*nodename ;	/* OS nodename (no domain) */
	const char	*domainname ;	/* INET domain */
	const char	*username ;	/* PASSWD username */
	const char	*password ;	/* PASSWD password */
	const char	*gecos ;	/* PASSWD GECOS field */
	const char	*homedname ;	/* PASSWD (home) directory */
	const char	*shell ;	/* PASSWD SHELL */
	const char	*organization ;	/* GECOS organization */
	const char	*gecosname ;	/* GECOS name */
	const char	*account ;	/* GECOS account */
	const char	*bin ;		/* GECOS printer-bin */
	const char	*office ;	/* GECOS office */
	const char	*wphone ;	/* GECOS work-phone */
	const char	*hphone ;	/* GECOS home-phone */
	const char	*printer ;	/* GECOS printer */
	const char	*realname ;	/* processed GECOS-name */
	const char	*mailname ;	/* best compacted mail-name */
	const char	*fullname ;	/* best fullname */
	const char	*name ;		/* best compacted name */
	const char	*groupname ;	/* login groupname */
	const char	*project ;	/* user project name */
	const char	*tz ;		/* user time-zone */
	const char	*md ;		/* mail-spool directory */
	const char	*wstation ;	/* user weather-station */
	const char	*logid ;	/* suggested ID for logging */
	const char	*a ;		/* memory allocation */
	USERINFO_FL	f ;
	pid_t		pid ;
	uid_t		uid, euid ;
	gid_t		gid, egid ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int userinfo_start(USERINFO *,const char *) ;
extern int userinfo_finish(USERINFO *) ;
extern int userinfo(USERINFO *,char *,int,const char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* USERINFO_INCLUDE */


