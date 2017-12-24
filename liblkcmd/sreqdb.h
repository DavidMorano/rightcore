/* sreqdb */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SREQDB_INCLUDE
#define	SREQDB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vechand.h>
#include	<intiq.h>
#include	<localmisc.h>

#include	"sreq.h"


#define	SREQDB		struct sreqdb_head
#define	SREQDB_MAGIC	0x75385212
#define	SREQDB_JOBIDLEN	SREQ_JOBIDLEN	/* same as LOGIDLEN? */


struct sreqdb_head {
	uint		magic ;
	cchar		*tmpdname ;
	vechand		db ;
	INTIQ		exits ;		/* thread-exits by job-ID (jsn) */
	volatile int	f_threxiting ;	/* a child thread is exiting */
} ;


#if	(! defined(SREQDB_MASTER)) || (SREQDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sreqdb_start(SREQDB *,cchar *,int) ;
extern int sreqdb_newjob(SREQDB *,int,cchar *,int,int) ;
extern int sreqdb_typeset(SREQDB *,int,int,int) ;
extern int sreqdb_get(SREQDB *,int,SREQ **) ;
extern int sreqdb_findpid(SREQDB *,pid_t,SREQ **) ;
extern int sreqdb_thrsdone(SREQDB *,SREQ **) ;
extern int sreqdb_del(SREQDB *,int) ;
extern int sreqdb_delobj(SREQDB *,SREQ *) ;
extern int sreqdb_findfd(SREQDB *,int) ;
extern int sreqdb_exiting(SREQDB *,int) ;
extern int sreqdb_count(SREQDB *) ;
extern int sreqdb_builtrelease(SREQDB *) ;
extern int sreqdb_finish(SREQDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SREQDB_MASTER */


#endif /* SREQDB_INCLUDE */


