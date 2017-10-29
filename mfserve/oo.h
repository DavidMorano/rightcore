/* sreq */
/* Service-Request */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SREQ_INCLUDE
#define	SREQ_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sockaddress.h>
#include	<localmisc.h>


#define	SREQ		struct sreq
#define	SREQ_JOBIDLEN	15			/* same as LOGIDLEN? */


struct sreq {
	SOCKADDRESS	sa ;			/* peername socket address */
	const char	*peername ;
	const char	*netuser ;
	const char	*netpass ;
	const char	*netident ;
	const char	*svcbuf ;		/* service-buffer */
	const char	*svc ;			/* allocated */
	const char	*subsvc ;
	char		*efname ;		/* Error-File-Name */
	time_t		stime ;			/* start time */
	time_t		atime ;			/* arrival? time */
	pid_t		pid ;			/* child PID */
	int		salen ;			/* socket length */
	int		nnames ;		/* number of names */
	int		ifd ;			/* file-descriptor input */
	int		ofd ;			/* output */
	int		efd ;			/* error */
	int		jtype ;			/* job-type */
	int		stype ;			/* sub-type */
	int		svclen ;		/* service-length */
	int		f_long ;		/* the "long" switch */
	int		f_local ;		/* client is local */
	char		jobid[SREQ_JOBIDLEN+1] ;
} ;


#if	(! defined(SREQ_MASTER)) || (SREQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sreq_start(SREQ *,cchar *,cchar *,int,int) ;
extern int sreq_typeset(SREQ *,int,int) ;
extern int sreq_getfd(SREQ *) ;
extern int sreq_havefd(SREQ *,int) ;
extern int sreq_svcadd(SREQ *,cchar *,int) ;
extern int sreq_setsvc(SREQ *,cchar *,int) ;
extern int sreq_setlong(SREQ *,int) ;
extern int sreq_getsvc(SREQ *,cchar **) ;
extern int sreq_getsubsvc(SREQ *,cchar **) ;
extern int sreq_finish(SREQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SREQ_MASTER */


#endif /* SREQ_INCLUDE */


