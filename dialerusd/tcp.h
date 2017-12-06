/* tcp */


#ifndef	TCP_INCLUDE
#define	TCP_INCLUDE	1


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	TCP_MAGIC	31415926
#define	TCP		struct tcp_head


struct tcp_head {
	unsigned long	magic ;
	int		fd ;
} ;


#if	(! defined(TCP_MASTER)) || (TCP_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int tcp_open(TCP *,SYSDIALER_ARGS *,cchar *,cchar *,cchar **) ;
extern int tcp_reade(TCP *,char *,int,int,int) ;
extern int tcp_recve(TCP *,char *,int,int,int,int) ;
extern int tcp_recvfrome(TCP *,char *,int,int,void *,int *,int,int) ;
extern int tcp_recvmsge(TCP *,struct msghdr *,int,int,int) ;
extern int tcp_write(TCP *,const char *,int) ;
extern int tcp_send(TCP *,const char *,int,int) ;
extern int tcp_sendto(TCP *,const char *,int,int,void *,int) ;
extern int tcp_sendmsg(TCP *,struct msghdr *,int) ;
extern int tcp_shutdown(TCP *,int) ;
extern int tcp_close(TCP *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TCP_MASTER */

#endif /* TCP_INCLUDE */


