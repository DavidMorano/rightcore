/* maintqotd (include-header) */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	MAINTQOTD_INCLUDE
#define	MAINTQOTD_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<vecpstr.h>
#include	<localmisc.h>		/* extra types */


#define	MAINTQOTD_SEARCHNAME	"maintqotd"
#define	MAINTQOTD_PRNAME	"LOCAL"
#define	MAINTQOTD_PROGEXEC	"qotd"
#define	MAINTQOTD_PROGMKQOTD	"helloworld"
#define	MAINTQOTD_CONFMAGIC	0x8932170
#define	MAINTQOTD		struct maintqotd_head
#define	MAINTQOTD_FL		struct maintqotd_flags


struct maintqotd_flags {
	uint		stores:1 ;
	uint		id:1 ;
	uint		hosts:1 ;
	uint		sources:1 ;
	uint		logsub:1 ;
	uint		logsize:1 ;
	uint		spooldir:1 ;
	uint		lfname:1 ;
	uint		to:1 ;
} ;

struct maintqotd_head {
	vecstr		stores ;
	vecpstr		hosts ;
	vecpstr		sources ;
	MAINTQOTD_FL	f, have, changed, final ;
	MAINTQOTD_FL	open ;
	IDS		id ;
	const char	*pr ;
	const char	*pn ;		/* program-name */
	const char	*sn ;		/* search-name */
	const char	*nn ;		/* node-name */
	const char	*dn ;		/* domain-name */
	const char	*un ;		/* user-name */
	const char	*logid ;	/* log-ID */
	const char	*lfname ;	/* log file-name */
	const char	*hostname ;
	const char	*spooldname ;	/* spool directory */
	const char	*qdname ;	/* particular quote directory */
	void		*config ;	/* configuration state */
	void		*logsub ;	/* log-file state */
	mode_t		om ;
	uid_t		euid ;
	uid_t		uid_pr ;
	gid_t		gid_pr ;
	time_t		dt ;
	int		of ;
	int		to ;
	int		mjd ;
	int		logsize ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

int maintqotd(const char *,int,int,int) ;

#ifdef	__cplusplus
}
#endif


#endif /* MAINTQOTD_INCLUDE */


