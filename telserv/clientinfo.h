/* clientinfo */


/* Copyright © 1999,2008 David A­D­ Morano.  All rights reserved. */


#ifndef	CLIENTINFO_INCLUDE
#define	CLIENTINFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<sockaddress.h>
#include	<vecstr.h>
#include	<localmisc.h>


#define	CLIENTINFO	struct clientinfo


struct clientinfo {
	SOCKADDRESS	sa ;			/* peername socket address */
	vecstr		names ;
	vecstr		stores ;
	const char	*peername ;
	const char	*netuser ;
	const char	*netpass ;
	const char	*netident ;
	const char	*service ;		/* service */
	const char	*subservice ;		/* subservice */
	long		mtype ;			/* SYSV-IPC message type */
	time_t		stime ;			/* start time */
	pid_t		pid ;
	int		salen ;			/* socket length */
	int		nnames ;		/* number of names */
	int		fd_input ;
	int		fd_output ;
	int		f_long ;		/* the "long" switch */
	int		f_local ;		/* client is local */
} ;

#ifdef	__cplusplus
extern "C" {
#endif

extern int clientinfo_start(struct clientinfo *) ;
extern int clientinfo_finish(struct clientinfo *) ;
extern int clientinfo_loadnames(struct clientinfo *,const char *) ;

#ifdef	__cplusplus
}
#endif


#endif /* CLIENTINFO_INCLUDE */


