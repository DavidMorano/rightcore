/* syshelper.h */

/* syshelper operations */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSHELPER_INCLUDE
#define	SYSHELPER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<vsystem.h>


/* object defines */

#define	SYSHELPER		struct syshelper_head


struct syshelper_head {
	uint	magic ;
	int	fd ;		/* socket file descriptor */
	int	pid ;		/* daemon PID */
} ;


typedef struct syshelper_head	syshelper ;


#ifndef	SYSHELPER_MASTER

#ifdef	__cplusplus
extern "C" {
#endif

extern int syshelper_start(SYSHELPER *,char *) ;
extern int syshelper_getpid(SYSHELPER *,pid_t *) ;
extern int syshelper_finish(SYSHELPER *) ;
extern int syshelper_localtime(SYSHELPER *,time_t,struct tm *) ;
extern int syshelper_random(SYSHELPER *,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSHELPER_MASTER */

#endif /* SYSHELPER_INCLUDE */


