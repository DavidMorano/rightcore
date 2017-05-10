/* thrcomm */

/* Thread-Communication (THRCOMM) */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	THRCOMM_INCLUDE
#define	THRCOMM_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<ptm.h>
#include	<ptc.h>
#include	<localmisc.h>		/* for unsigned types */


#define	THRCOMM		struct thrcomm_head
#define	THRCOMM_MAGIC	0x26293177


struct thrcomm_head {
	uint		magic ;
	PTM		m ;
	PTC		c ;
	volatile int	cmd ;
	volatile int	rrs ;
	volatile int	f_cmd ;
	volatile int	f_exiting ;
} ;


#if	(! defined(THRCOMM_MASTER)) || (THRCOMM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	thrcomm_start(THRCOMM *,int) ;
extern int	thrcomm_cmdsend(THRCOMM *,int,int) ;
extern int	thrcomm_cmdrecv(THRCOMM *,int) ;
extern int	thrcomm_rspsend(THRCOMM *,int,int) ;
extern int	thrcomm_rsprecv(THRCOMM *,int) ;
extern int	thrcomm_exiting(THRCOMM *) ;
extern int	thrcomm_finish(THRCOMM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* THRCOMM_MASTER */

#endif /* THRCOMM_INCLUDE */


