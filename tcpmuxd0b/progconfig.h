/* progconfig */

/* program configuration code */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	PROGCONFIG_INCLUDE
#define	PROGCONFIG_INCLUDE	1


#include	<sys/types.h>

#include	<expcook.h>
#include	<paramfile.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


#define	PROGCONFIG	struct progconfig_head


struct progconfig_flags {
	uint		srvtab:1 ;	/* do we have an SRVTAB ? */
	uint		acctab:1 ;	/* do we have an ACCess TABle ? */
	uint		passwd:1 ;	/* PWFILE? */
	uint		pidfile:1 ;
	uint		lockfile:1 ;	/* have a lock file */
	uint		mspoll:1 ;	/* do MS polling */
	uint		zerospeed:1 ;	/* zero out 'speed' element */
	uint		nopass:1 ;	/* don't listen on PASS */
	uint		msfile:1 ;
	uint		logfile:1 ;
	uint		loglen:1 ;
	uint		lockint:1 ;
	uint		runint:1 ;
	uint		pollint:1 ;
	uint		speedint:1 ;
	uint		markint:1 ;
	uint		cmd:1 ;
	uint		p:1 ;		/* paramfile */
	uint		lockinfo:1 ;
	uint		portspec:1 ;
} ;

struct progconfig_head {
	unsigned long	magic ;
	const char	*configfname ;
	const char	*pidfname ;
	const char	*lockfname ;		/* lock file */
	const char	*svcfname ;		/* SVCTAB file */
	const char	*accfname ;		/* ACCTAB file */
	const char	*passfname ;		/* pass (FD) file */
	const char	*reqfname ;		/* request file */
	const char	*shmfname ;		/* SHM file */
	const char	*msfname ;		/* MS file */
	const char	*logfname ;
	const char	*prog_rmail ;
	const char	*prog_sendmail ;
	const char	*orgcode ;		/* organization code */
	const char	*speedname ;		/* CPUSPEED module name */
	const char	*portspec ;
	struct proginfo	*pip ;
	struct progconfig_flags	f ;
	struct progconfig_flags	have ;
	struct progconfig_flags	change ;
	struct progconfig_flags	open ;
	struct progconfig_flags	final ;
	PARAMFILE	p ;
	EXPCOOK	cooks ;
	vecstr		stores ;
} ;


#if	(! defined(PROGCONFIG_MASTER)) || (PROGCONFIG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int progconfigstart(PROGCONFIG *,struct proginfo *,const char *) ;
extern int progconfigcheck(PROGCONFIG *) ;
extern int progconfigread(PROGCONFIG *) ;
extern int progconfigfinish(PROGCONFIG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROGCONFIG_MASTER */

#endif /* PROGCONFIG */



