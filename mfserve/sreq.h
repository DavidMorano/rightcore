/* sreq */
/* Service-Request */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SREQ_INCLUDE
#define	SREQ_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"mfslocinfo.h"
#include	"svcentsub.h"


#define	SREQ		struct sreq
#define	SREQ_FL		struct sreq_flags
#define	SREQ_JOBIDLEN	15			/* same as LOGIDLEN? */


enum sreqstates {
	sreqstate_acquire,		/* acquire service string */
	sreqstate_svc,			/* have service string */
	sreqstate_prog,			/* spawned program (process) */
	sreqstate_thread,		/* spawned thread */
	sreqstate_done,			/* job finished */
	sreqstate_overlast
} ;

struct sreq_flags {
	uint		process:1 ;
	uint		thread:1 ;
	uint		local:1 ;		/* client is local */
	uint		longopt:1 ;		/* the "long" switch */
	uint		ss:1 ;
} ;

struct sreq {
	SREQ_FL		f ;
	SOCKADDRESS	sa ;			/* peername socket address */
	SVCENTSUB	ss ;
	const char	*peername ;
	const char	*netuser ;
	const char	*netpass ;
	const char	*netident ;
	const char	*svcbuf ;		/* service-buffer */
	const char	*svc ;			/* allocated */
	const char	*subsvc ;
	char		*efname ;		/* Error-File-Name */
	time_t		stime ;			/* time-start */
	time_t		etime ;			/* time-end */
	pid_t		pid ;			/* child PID */
	pthread_t	tid ;			/* child thread ID */
	volatile int	f_exiting ;
	int		ji ;			/* job number in DB */
	int		jsn ;			/* job serial number */
	int		salen ;			/* peername socket length */
	int		nnames ;		/* number of names */
	int		ifd ;			/* file-descriptor input */
	int		ofd ;			/* output */
	int		efd ;			/* error */
	int		jtype ;			/* job-type */
	int		stype ;			/* sub-type */
	int		state ;			/* job state (see above) */
	int		svclen ;		/* service-length */
	int		trs ;			/* thread return-status */
	char		logid[SREQ_JOBIDLEN+1] ;
} ;


#if	(! defined(SREQ_MASTER)) || (SREQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sreq_start(SREQ *,cchar *,cchar *,int,int) ;
extern int sreq_typeset(SREQ *,int,int) ;
extern int sreq_getfd(SREQ *) ;
extern int sreq_havefd(SREQ *,int) ;
extern int sreq_addsvc(SREQ *,cchar *,int) ;
extern int sreq_setsvc(SREQ *,int) ;
extern int sreq_setlong(SREQ *,int) ;
extern int sreq_setstate(SREQ *,int) ;
extern int sreq_getsvc(SREQ *,cchar **) ;
extern int sreq_getsubsvc(SREQ *,cchar **) ;
extern int sreq_getstate(SREQ *) ;
extern int sreq_ofd(SREQ *) ;
extern int sreq_svcentbegin(SREQ *,LOCINFO *,SVCENT *) ;
extern int sreq_svcentend(SREQ *) ;
extern int sreq_thrdone(SREQ *) ;
extern int sreq_finish(SREQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SREQ_MASTER */


#endif /* SREQ_INCLUDE */


