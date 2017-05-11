/* uterm */

/* object to handle UNIX terminal stuff */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	UTERM_INCLUDE
#define	UTERM_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<termios.h>
#include	<limits.h>

#include	<charq.h>
#include	<localmisc.h>		/* extra types */


#define	UTERM		struct uterm_head
#define	UTERM_PROMPT	struct uterm_prompt
#define	UTERM_LOAD	struct uterm_load
#define	UTERM_FL	struct uterm_flags

enum utermcmds {
	utermcmd_noop,
	utermcmd_getuid,
	utermcmd_getsid,
	utermcmd_getmode,
	utermcmd_setmode,
	utermcmd_setrterms,
	utermcmd_reestablish,
	utermcmd_getmesg,
	utermcmd_setmesg,
	utermcmd_getbiff,
	utermcmd_setbiff,
	utermcmd_getlines,
	utermcmd_setlines,
	utermcmd_getpgrp,
	utermcmd_setpgrp,
	utermcmd_getpop,
	utermcmd_setpop,
	utermcmd_overlast
} ;

struct uterm_flags {
	uint		co:1 ;		/* control-O */
	uint		cc:1 ;		/* control-C */
	uint		cy:1 ;		/* control-Y */
	uint		cz:1 ;		/* control-Z */
	uint		dle:1 ;		/* data-link-escape */
	uint		rw:1 ;
	uint		suspend:1 ;	/* output suspended */
	uint		read:1 ;	/* read in progress */
	uint		nosig:1 ;	/* no-signal-generation mode */
	uint		nosigecho:1 ;	/* no echo for signals */
	uint		noctty:1 ;	/* the terminal is not controlling */
	uint		noflow:1 ;	/* no output flow control */
} ;

/* prompt-output before input */
struct uterm_prompt {
	const char	*pbuf ;
	int		plen ;
} ;

/* pre-loading the input buffer */
struct uterm_load {
	const char	*lbuf ;
	int		llen ;
} ;

struct uterm_head {
	uint		magic ;
	UTERM_FL	f ;
	struct termios	ts_old ;
	struct termios	ts_new ;
	CHARQ		taq ;
	CHARQ		ecq ;
	time_t		ti_start ;
	uid_t		uid ;
	int		fd ;
	int		loopcount ;
	int		timeout ;	/* timeout timer counter */
	int		mode ;
	int		ch_read, ch_write ;
	int		stat ;
	uchar		rterms[32] ;
} ;


#if	(! defined(UTERM_MASTER)) || (UTERM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uterm_start(UTERM *,int) ;
extern int uterm_control(UTERM *,int,...) ;
extern int uterm_status(UTERM *,int,...) ;
extern int uterm_read(UTERM *,char *,int) ;
extern int uterm_reade(UTERM *,char *,int,int,int,UTERM_PROMPT *,UTERM_LOAD *) ;
extern int uterm_write(UTERM *,const char *,int) ;
extern int uterm_suspend(UTERM *) ;
extern int uterm_resume(UTERM *) ;
extern int uterm_restore(UTERM *) ;
extern int uterm_ensure(UTERM *) ;
extern int uterm_getmesg(UTERM *) ;
extern int uterm_getbiff(UTERM *) ;
extern int uterm_getpop(UTERM *) ;
extern int uterm_setpop(UTERM *,int) ;
extern int uterm_finish(UTERM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UTERM_MASTER */

#endif /* UTERM_INCLUDE */


