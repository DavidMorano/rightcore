/* defs */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecstr.h>
#include	<bfile.h>
#include	<localmisc.h>



#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	ZNAMELEN
#define	ZNAMELEN	8
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


struct proginfo_flags {
	uint		progdash:1 ;	/* leading dash on program-name */
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		ignore:1 ;
	uint		errfile:1 ;
	uint		outfile:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;	/* execution filename */
	const char	*progdname ;	/* dirname of arg[0] */
	const char	*progname ;	/* basename of arg[0] */
	const char	*pr ;		/* program root */
	const char	*rootname ;	/* distribution name */
	const char	*searchname ;
	const char	*version ;
	const char	*banner ;
	const char	*usysname ;	/* UNAME OS system-name */
	const char	*umachine ;	/* UNAME machine name */
	const char	*urelease ;	/* UNAME OS release */
	const char	*uversion ;	/* UNAME OS version */
	const char	*architecture ;	/* UAUX machine architecture */
	const char	*platform ;	/* UAUX machine platform */
	const char	*provider ;	/* UAUX machine provider */
	const char	*hz ;		/* OS HZ */
	const char	*nodename ;	/* USERINFO */
	const char	*domainname ;	/* USERINFO */
	const char	*username ;	/* USERINFO */
	const char	*homedname ;	/* USERINFO */
	const char	*shell ;	/* USERINFO */
	const char	*org ;		/* USERINFO */
	const char	*gecosname ;	/* USERINFO */
	const char	*realname ;	/* USERINFO */
	const char	*name ;		/* USERINFO */
	const char	*fullname ;	/* USERINFO full-name */
	const char	*mailname ;	/* USERINFO mail-abbreviated-name */
	const char	*helpfname ;
	bfile		*efp ;
	struct proginfo_flags	f ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	const char	zname[ZNAMELEN + 1] ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;

struct arginfo {
	int		argc ;
	int		ai, ai_max, ai_pos ;
	const char	**argv ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(struct proginfo *,const char **,
		const char *,const char *) ;
extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_setexecname(struct proginfo *,const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_rootname(struct proginfo *) ;
extern int proginfo_progdname(struct proginfo *) ;
extern int proginfo_progename(struct proginfo *) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_getenv(struct proginfo *,const char *,int,const char **) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


