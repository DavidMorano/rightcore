/* tcpnls */


#ifndef	TCPNLS_INCLUDE
#define	TCPNLS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	TCPNLS_MAGIC	31415926
#define	TCPNLS		struct tcpnls_head


struct tcpnls_head {
	uint		magic ;
	time_t		opentime ;
	int		fd ;
} ;


#if	(! defined(TCPNLS_MASTER)) || (TCPNLS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int tcpnls_open(TCPNLS *,SYSDIALER_ARGS *,
		const char *,const char *,const char **) ;
extern int tcpnls_reade(TCPNLS *,char *,int,int,int) ;
extern int tcpnls_recve(TCPNLS *,char *,int,int,int,int) ;
extern int tcpnls_recvfrome(TCPNLS *,char *,int,int,void *,int *,int,int) ;
extern int tcpnls_recvmsge(TCPNLS *,struct msghdr *,int,int,int) ;
extern int tcpnls_write(TCPNLS *,const char *,int) ;
extern int tcpnls_send(TCPNLS *,const char *,int,int) ;
extern int tcpnls_sendto(TCPNLS *,const char *,int,int,void *,int) ;
extern int tcpnls_sendmsg(TCPNLS *,struct msghdr *,int) ;
extern int tcpnls_shutdown(TCPNLS *,int) ;
extern int tcpnls_close(TCPNLS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TCPNLS_MASTER */

#endif /* TCPNLS_INCLUDE */


