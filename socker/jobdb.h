/* jobdb */

/* last modified %G% version %I% */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	JOBDB_INCLUDE
#define	JOBDB_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecitem.h>
#include	<localmisc.h>


#define	JOBDB		struct jobdb_head
#define	JOBDB_ENT	struct jobdb_ent

#define	JOBDB_JOBIDLEN	15			/* same as LOGIDLEN? */
#define	JOBDB_JOBPREFIX	"jobdb"			/* job-file prefix */
#define	JOBDB_JOBFILETO	(5*3600)		/* job-file time-out */


struct jobdb_ent {
	const char	*name ;
	char		*ofname ;
	char		*efname ;
	time_t		atime ;			/* job arrival time */
	time_t		stime ;			/* job start time */
	pid_t		pid ;			/* run flag */
	char		jobid[JOBDB_JOBIDLEN + 1] ;
} ;

struct jobdb_head {
	const char	*tmpdname ;
	VECITEM		db ;
	time_t		ti_jobdir ;
} ;


#if	(! defined(JOBDB_MASTER)) || (JOBDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int jobdb_start(JOBDB *,int,cchar *) ;
extern int jobdb_newjob(JOBDB *,cchar *,int) ;
extern int jobdb_get(JOBDB *,int,JOBDB_ENT **) ;
extern int jobdb_findpid(JOBDB *,pid_t,JOBDB_ENT **) ;
extern int jobdb_del(JOBDB *,int) ;
extern int jobdb_delp(JOBDB *,JOBDB_ENT *) ;
extern int jobdb_count(JOBDB *) ;
extern int jobdb_check(JOBDB *,time_t,int) ;
extern int jobdb_finish(JOBDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* JOBDB_MASTER */


#endif /* JOBDB_INCLUDE */


