/* msgbuf */

/* message-buffering */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MSGBUF_INCLUDE
#define	MSGBUF_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* object defines */

#define	MSGBUF		struct msgbuf


struct msgbuf {
	char		*buf ;
	char		*bp ;
	int		fd ;
	int		to ;
	int		bufsize ;
	int		bl ;
	int		neof ;
} ;


#if	(! defined(MSGBUF_MASTER)) || (MSGBUF_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	msgbuf_start(MSGBUF *,int,int,int) ;
extern int	msgbuf_read(MSGBUF *,cchar **) ;
extern int	msgbuf_adv(MSGBUF *,int) ;
extern int	msgbuf_update(MSGBUF *,int) ;
extern int	msgbuf_finish(MSGBUF *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MSGBUF_MASTER */

#endif /* MSGBUF_INCLUDE */


