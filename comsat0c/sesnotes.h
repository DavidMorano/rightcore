/* sesnotes */


/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

#ifndef	SESNOTES_INCLUDE
#define	SESNOTES_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sockaddress.h>
#include	<localmisc.h>


#define	SESNOTES		struct sesnotes_head
#define	SESNOTES_MAGIC		0x04431590
#define	SESNOTES_SESDNAME	"/var/tmp/sessions"


struct sesnotes_head {
	uint		magic ;
	char		*sfname ;
	pid_t		pid ;
	int		fd ;
	char		unbuf[USERNAMELEN+1] ;
} ;


#if	(! defined(SESNOTES_MASTER)) || (SESNOTES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sesnotes_open(SESNOTES *,const char *) ;
extern int sesnotes_send(SESNOTES *,int,const char *,int,pid_t) ;
extern int sesnotes_sendbiff(SESNOTES *,const char *,int,pid_t) ;
extern int sesnotes_sendgen(SESNOTES *,const char *,int,pid_t) ;
extern int sesnotes_close(SESNOTES *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SESNOTES_MASTER */

#endif /* SESNOTES_INCLUDE */


