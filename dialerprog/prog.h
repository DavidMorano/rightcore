/* prog */


/* revision history:

	= 2003-11-04, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	PROG_INCLUDE
#define	PROG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<logfile.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	PROG		struct prog_head


struct prog_flags {
	uint		log:1 ;
} ;

struct prog_head {
	unsigned long	magic ;
	struct prog_flags	f ;
	LOGFILE		lh ;
	pid_t		pid ;
	int		fd ;
	int		tlen ;
	char		efname[MAXPATHLEN + 1] ;
} ;


#if	(! defined(PROG_MASTER)) || (PROG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int prog_open(PROG *,SYSDIALER_ARGS *,
		const char *,const char *,const char **) ;
extern int prog_reade(PROG *,char *,int,int,int) ;
extern int prog_recve(PROG *,char *,int,int,int,int) ;
extern int prog_recvfrome(PROG *,char *,int,int,void *,int *,int,int) ;
extern int prog_recvmsge(PROG *,struct msghdr *,int,int,int) ;
extern int prog_write(PROG *,const char *,int) ;
extern int prog_send(PROG *,const char *,int,int) ;
extern int prog_sendto(PROG *,const char *,int,int,void *,int) ;
extern int prog_sendmsg(PROG *,struct msghdr *,int) ;
extern int prog_shutdown(PROG *,int) ;
extern int prog_close(PROG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROG_MASTER */

#endif /* PROG_INCLUDE */


