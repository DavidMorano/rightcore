/* pcsnsc */

/* PCS Name-Server-Client */


/* revision history:

	= 2000-12-18, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSNSC_INCLUDE
#define	PCSNSC_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<realname.h>
#include	<pcsnsreq.h>
#include	<localmisc.h>


#define	PCSNSC_MAGIC	0x58261221
#define	PCSNSC		struct pcsnsc_head
#define	PCSNSC_OBJ	struct pcsnsc_obj
#define	PCSNSC_FL	struct pcsnsc_flags
#define	PCSNSC_STATUS	struct pcsnsc_status


struct pcsnsc_obj {
	char		*name ;
	uint		objsize ;
} ;

struct pcsnsc_status {
	pid_t		pid ;
	uint		queries ;		/* server is present */
} ;

struct pcsnsc_flags {
	uint		srv:1 ;		/* server is present */
} ;

struct pcsnsc_head {
	uint		magic ;
	const char	*pr ;
	const char	*srcfname ;
	const char	*srvfname ;
	char		*mbuf ;
	PCSNSC_FL	f ;
	pid_t		pid ;
	int		mlen ;
	int		fd ;
	int		to ;
} ;


#if	(! defined(PCSNSC_MASTER)) || (PCSNSC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsnsc_open(PCSNSC *,cchar *,int) ;
extern int pcsnsc_status(PCSNSC *,PCSNSC_STATUS *) ;
extern int pcsnsc_help(PCSNSC *,char *,int,int) ;
extern int pcsnsc_getval(PCSNSC *,char *,int,cchar *,int) ;
extern int pcsnsc_getname(PCSNSC *,char *,int,cchar *) ;
extern int pcsnsc_mark(PCSNSC *) ;
extern int pcsnsc_exit(PCSNSC *,cchar *) ;
extern int pcsnsc_close(PCSNSC *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSNSC_MASTER */

#endif /* PCSNSC_INCLUDE */


