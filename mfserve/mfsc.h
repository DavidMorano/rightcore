/* mfsc */

/* PCS Name-Server-Client */


/* revision history:

	= 2000-12-18, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSC_INCLUDE
#define	MFSC_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#define	MFSC_MAGIC	0x58261222
#define	MFSC		struct mfsc_head
#define	MFSC_OBJ	struct mfsc_obj
#define	MFSC_FL		struct mfsc_flags
#define	MFSC_STATUS	struct mfsc_status


struct mfsc_obj {
	char		*name ;
	uint		objsize ;
} ;

struct mfsc_status {
	pid_t		pid ;
	uint		queries ;		/* server is present */
} ;

struct mfsc_flags {
	uint		srv:1 ;		/* server is present */
} ;

struct mfsc_head {
	uint		magic ;
	const char	*pr ;
	const char	*srcfname ;
	const char	*srvfname ;
	char		*mbuf ;
	MFSC_FL		f ;
	pid_t		pid ;
	int		mlen ;
	int		fd ;
	int		to ;
} ;


#if	(! defined(MFSC_MASTER)) || (MFSC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mfsc_open(MFSC *,cchar *,int) ;
extern int mfsc_status(MFSC *,MFSC_STATUS *) ;
extern int mfsc_help(MFSC *,char *,int,int) ;
extern int mfsc_getval(MFSC *,char *,int,cchar *,int) ;
extern int mfsc_getname(MFSC *,char *,int,cchar *) ;
extern int mfsc_mark(MFSC *) ;
extern int mfsc_exit(MFSC *,cchar *) ;
extern int mfsc_close(MFSC *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSC_MASTER */

#endif /* MFSC_INCLUDE */


