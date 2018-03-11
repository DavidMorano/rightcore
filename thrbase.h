/* thrbase */

/* access manager interface to PCS user-mode polling */


/* revision history:

	= 2008-10-07, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	THRBASE_INCLUDE
#define	THRBASE_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<thrcomm.h>
#include	<localmisc.h>


#define	THRBASE		struct thrbase_head
#define	THRBASE_SI	struct thrbase_startinfo
#define	THRBASE_INFO	struct thrbase_i
#define	THRBASE_MAGIC	0x88773423


struct thrbase_i {
	int		dummy ;
} ;

struct thrbase_startinfo {
	THRBASE		*tip ;
	int		(*worker)(THRBASE *,void *) ;
} ;

struct thrbase_head {
	void		*ap ;
	THRBASE_SI	*sip ;
	THRCOMM		tc ;
	pthread_t	tid ;
	volatile int	trs ;
	volatile int	f_exiting ;
	int		f_exited ;
} ;

enum thrbasecmds {
	thrbasecmd_noop,
	thrbasecmd_exit,
	thrbasecmd_overlast
} ;


#if	(! defined(THRBASE_MASTER)) || (THRBASE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	thrbase_start(THRBASE *,int (*)(THRBASE *,void *),void *) ;
extern int	thrbase_cmdsend(THRBASE *,int,int) ;
extern int	thrbase_cmdexit(THRBASE *) ;
extern int	thrbase_cmdrecv(THRBASE *,int) ;
extern int	thrbase_exiting(THRBASE *) ;
extern int	thrbase_waitexit(THRBASE *) ;
extern int	thrbase_info(THRBASE *,THRBASE_INFO *) ;
extern int	thrbase_finish(THRBASE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* THRBASE_MASTER */

#endif /* THRBASE_INCLUDE */


