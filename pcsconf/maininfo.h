/* maininfo */

/* last modified %G% version %I% */


/* revision history:

	= 2004-05-05, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#ifndef	MAININFO_INCLUDE
#define	MAININFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<pthread.h>

#include	<vecstr.h>
#include	<localmisc.h>

#include	"sighand.h"


#define	MAININFO	struct maininfo
#define	MAININFO_FL	struct maininfo_flags


typedef void		(*maininfohand_t)(int,siginfo_t *,void *) ;


struct maininfo_flags {
	uint		progdash:1 ;	/* leading dash on program-name */
	uint		utilout:1 ;	/* utility is out running */
} ;

struct maininfo {
	SIGHAND		sh ;
	vecstr		stores ;
	stack_t		astack ;
	const char	*progdname ;
	const char	*progename ;
	const char	*progname ;
	const char	*srchname ;
	const char	*symname ;
	void		*mdata ;
	MAININFO_FL	have, f, changed, final ;
	MAININFO_FL	open ;
	sigset_t	savemask ;
	pthread_t	tid ;
	size_t		msize ;
	volatile int	f_done ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int maininfo_start(MAININFO *,int,cchar **) ;
extern int maininfo_finish(MAININFO *) ;
extern int maininfo_setentry(MAININFO *,cchar **,cchar *,int) ;
extern int maininfo_sigbegin(MAININFO *,maininfohand_t,const int *) ;
extern int maininfo_sigend(MAININFO *) ;
extern int maininfo_utilbegin(MAININFO *,int) ;
extern int maininfo_utilend(MAININFO *) ;
extern int maininfo_srchname(MAININFO *,cchar **) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAININFO_INCLUDE */


