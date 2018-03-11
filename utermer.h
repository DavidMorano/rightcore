/* utermer */

/* object to handle UNIX terminal stuff */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	UTERMER_INCLUDE
#define	UTERMER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<termios.h>
#include	<limits.h>
#include	<pthead.h>
#include	<time.h>

#include	<charq.h>
#include	<ptm.h>
#include	<psem.h>
#include	<piq.h>
#include	<ciq.h>
#include	<localmisc.h>


#define	UTERMER_MAGIC	0x33442281
#define	UTERMER		struct utermer_head
#define	UTERMER_PROMPT	struct utermer_prompt
#define	UTERMER_LOAD	struct utermer_load
#define	UTERMER_CS	struct utermer_cs


struct utermer_bdesc {
	char		*dbuf ;
	int		dlen ;
} ;

struct utermer_cs {
	PSEM		*sem ;
	int		len ;
	int		cs ;
} ;

struct utermer_flags {
	uint		co:1 ;		/* control O */
	uint		cc:1 ;		/* control C */
	uint		rw:1 ;
	uint		suspend:1 ;	/* output suspended */
	uint		read:1 ;	/* read in progress */
	uint		onint:1 ;	/* on an interrupt */
} ;

struct utermer_head {
	uint		magic ;
	struct utermer_flags	f ;
	struct termios	ts_old ;
	struct termios	ts_new ;
	PTM		mout ;
	CHARIQ		taq ;		/* type-ahead */
	CHARIQ		ecq ;		/* echo */
	PIQ		fstore ;	/* free-packet store */
	CIQ		rq ;		/* reader queue */
	CIQ		wq ;		/* writer queue */
	PSEM		rq_sem ;
	PSEM		wq_sem ;
	pthread_t	tid_read, tid_write, tid_int ;
	time_t		basetime ;
	int		fd ;
	int		loopcount ;
	int		timeout ;	/* timeout timer counter */
	int		mode ;
	int		ch_read, ch_write ;
	int		stat ;
} ;


#if	(! defined(UTERMER_MASTER)) || (UTERMER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int utermer_start(UTERMER *,int) ;
extern int utermer_control(UTERMER *,int,...) ;
extern int utermer_status(UTERMER *,int,...) ;
extern int utermer_read(UTERMER *,char *,int) ;
extern int utermer_reade(UTERMER *,char *,int,int,int,
		UTERMER_PROMPT *,UTERMER_LOAD *) ;
extern int utermer_reader(UTERMER *,UTERMER_CS *,char *,int,int,int,
		UTERMER_BDESC *,UTERMER_BDESC *) ;
extern int utermer_write(UTERMER *,const char *,int) ;
extern int utermer_suspend(UTERMER *) ;
extern int utermer_resume(UTERMER *) ;
extern int utermer_restore(UTERMER *) ;
extern int utermer_ensure(UTERMER *) ;
extern int utermer_finish(UTERMER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UTERMER_MASTER */

#endif /* UTERMER_INCLUDE */


