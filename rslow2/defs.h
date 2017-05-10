/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


#define	LINELEN		256		/* size of input line */
#define	BUFLEN		((MAXPATHLEN * 4) + 1)
#define	BUFSIZE		BUFLEN
#define	SAVESIZE	128


struct proginfo_flags {
	uint	sysvct : 1 ;
	uint	sysvrt : 1 ;
	uint	log : 1 ;
	uint	errout : 1 ;
	uint	quiet : 1 ;
} ;

struct proginfo {
	struct gflags	f ;
	logfile	lh ;
	bfile	*efp ;			/* error Basic file */
	char	*progname ;		/* program name */
	char	*programroot ;		/* program root directory */
	char	*logfname ;		/* the log file name */
	char	*logid ;		/* log ID for log file entries */
	char	*nodename ;		/* machine node name */
	char	*domainname ;		/* machine INET domain name */
	char	*username ;		/* username (not a real name) */
	char	*homedir ;		/* user's HOME directory */
	char	*timestring ;		/* current time string (CTIME) */
	char	*gecosname ;		/* straight GECOS name from 'passwd' */
	char	*mailname ;		/* massaged user's email name */
	char	*mailhost ;
	char	*fromnode ;
	char	*authusername ;
	char	*authpassword ;
	char	*netfname ;		/* user's NETRC file */
	time_t	daytime ;		/* system time of day */
	pid_t	pid ;
	uid_t	uid ;
	gid_t	gid ;
	uid_t	euid ;
	int	debuglevel ;
	int	verboselevel ;
	int	f_alarm ;		/* interrupt semaphore */
} ;



/* possible transport mechanisms */

#define	TRANS_NONE	0
#define	TRANS_CP	1
#define	TRANS_RFILE	2
#define	TRANS_UUCP	3
#define	TRANS_RCP	4


static char	*trans_name[] = {
	"NONE",
	"CP",
	"RFILE",
	"UUCP",
	"RCP",
	NULL
} ;


#endif /* DEFS_INCLUDE */


