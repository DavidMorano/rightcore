/* ucb */

/* Unit Control Block (a "driver" thing) */


#ifndef	UCB_INCLUDE
#define	UCB_INCLUDE		1


/* revision history:

	= 1998-11-01, David A­D­ Morano

	Originally written for Audix Database Processor (DBP) work.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<sys/types.h>
#include	<termios.h>
#include	<time.h>

#include	<charq.h>
#include	<localmisc.h>


#define	UCB		struct ucb


struct ucb {
	ulong		magic ;
	struct termios	ts_old ;
	struct termios	ts_new ;
	CHARQ		taq ;
	CHARQ		ecq ;
	time_t		basetime ;
	int		fd ;
	int		loopcount ;
	int		timeout ;	/* timeout timer counter */
	int		mode ;
	short		stat ;
	char		rc, wc ;	/* read & write character storage */
	char		f_co ;		/* control O flag */
	char		f_cc ;		/* control C flag */
	char		f_rw ;		/* reader wakeup */
	long		unused[20] ;
} ;


#if	(! defined(UCB_MASTER)) || (UCB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ucb_init(UCB *,int) ;
extern int	ucb_free(UCB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* ICB_MASTER */


#endif /* UCB_INCLUDE */



