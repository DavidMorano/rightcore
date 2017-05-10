/* egs */

/* entropy-gathering-server operations */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	EGS_INCLUDE
#define	EGS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<vsystem.h>


/* object defines */

#define	EGS_DEFFILE	"/tmp/entropy"
#define	EGS_MAGIC	0x93847561
#define	EGS		struct egs_head


struct egs_head {
	uint		magic ;
	pid_t		pid ;		/* daemon PID */
	int		fd ;		/* socket file descriptor */
} ;


#if	(! defined(EGS_MASTER)) || (EGS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int egs_open(EGS *,const char *) ;
extern int egs_read(EGS *,char *,int) ;
extern int egs_write(EGS *,char *,int) ;
extern int egs_level(EGS *) ;
extern int egs_getpid(EGS *,pid_t *) ;
extern int egs_close(EGS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* EGS_MASTER */

#endif /* EGS_INCLUDE */


