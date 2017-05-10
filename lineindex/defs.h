/* defs (include-file) */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	LINELEN		2048		/* size of input line */

#define	BUFLEN		8192

#define	EOP		(~ 0)		/* end of post */



/* program information */

struct proginfo_flags {
	uint	progdash : 1 ;
	uint	akopts : 1 ;
	uint	aparams : 1 ;
	uint	quiet : 1 ;
	uint	log : 1 ;
	uint	stderror : 1 ;
	uint	removelabel : 1 ;
	uint	wholefile : 1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	char		*pwd ;
	char		*progdname ;	/* program directory */
	char		*progename ;	/* program directory */
	char		*progname ;	/* program name */
	char		*pr ;		/* program root directory */
	char		*searchname ;
	char		*version ;
	char		*banner ;
	char		*nodename ;
	char		*domainname ;
	char		*username ;
	char		*groupname ;
	char		*logid ;
	char		*tmpdname ;	/* temporary directory */
	void		*efp ;		/* error Basic file */
	struct proginfo_flags	f ;
	struct proginfo_flags	open ;
	logfile		lh ;
	time_t		daytime ;
	int		pwdlen ;
	int		debuglevel ;	/* debugging level */
	int		verboselevel ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(struct proginfo *,char **,const char *,const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_rootprogdname(struct proginfo *) ;
extern int proginfo_rootexecname(struct proginfo *,const char *) ;
extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_setexecname(struct proginfo *,const char *) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


