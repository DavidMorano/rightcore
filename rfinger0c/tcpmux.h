/* tcpmux */


#ifndef	TCPMUX_INCLUDE
#define	TCPMUX_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	TCPMUX_MAGIC	31415926
#define	TCPMUX		struct tcpmux_head


struct tcpmux_head {
	unsigned long	magic ;
	int		fd ;
} ;


#if	(! defined(TCPMUX_MASTER)) || (TCPMUX_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int tcpmux_open(TCPMUX *,SYSDIALER_ARGS *,
		const char *,const char *,const char **) ;
extern int tcpmux_reade(TCPMUX *,char *,int,int,int) ;
extern int tcpmux_recve(TCPMUX *,char *,int,int,int,int) ;
extern int tcpmux_recvfrome(TCPMUX *,char *,int,int,void *,int *,int,int) ;
extern int tcpmux_recvmsge(TCPMUX *,struct msghdr *,int,int,int) ;
extern int tcpmux_write(TCPMUX *,const char *,int) ;
extern int tcpmux_send(TCPMUX *,const char *,int,int) ;
extern int tcpmux_sendto(TCPMUX *,const char *,int,int,void *,int) ;
extern int tcpmux_sendmsg(TCPMUX *,struct msghdr *,int) ;
extern int tcpmux_shutdown(TCPMUX *,int) ;
extern int tcpmux_close(TCPMUX *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TCPMUX_MASTER */

#endif /* TCPMUX_INCLUDE */


