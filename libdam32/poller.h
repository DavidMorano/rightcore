/* poller */


/* revision history:

	= 2006-09-10, David A­D­ Morano
        I created this from hacking something that was similar that was
        originally created for a PCS program.

*/

/* Copyright © 2006 David A­D­ Morano.  All rights reserved. */

#ifndef	POLLER_INCLUDE
#define	POLLER_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<poll.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	POLLER_MAGIC	0x09854123
#define	POLLER		struct poller_head
#define	POLLER_SPEC	struct pollfd
#define	POLLER_CUR	struct poller_c


struct poller_c {
	int		i ;
} ;

struct poller_head {
	uint		magic ;
	vecobj		regs ;
	struct pollfd	*pa ;
	int		n ;		/* array number */
	int		e ;		/* array extent */
	int		nready ;
} ;


#if	(! defined(POLLER_MASTER)) || (POLLER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int poller_start(POLLER *) ;
extern int poller_reg(POLLER *,POLLER_SPEC *) ;
extern int poller_cancel(POLLER *,POLLER_SPEC *) ;
extern int poller_cancelfd(POLLER *,int) ;
extern int poller_wait(POLLER *,POLLER_SPEC *,int) ;
extern int poller_get(POLLER *,POLLER_SPEC *) ;
extern int poller_curbegin(POLLER *,POLLER_CUR *) ;
extern int poller_enum(POLLER *,POLLER_CUR *,POLLER_SPEC *) ;
extern int poller_curend(POLLER *,POLLER_CUR *) ;
extern int poller_finish(POLLER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* POLLER_MASTER */

#endif /* POLLER_INCLUDE */


