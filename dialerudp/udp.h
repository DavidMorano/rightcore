/* udp */


#ifndef	UDP_INCLUDE
#define	UDP_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	UDP_MAGIC	31415926
#define	UDP		struct udp_head


struct udp_head {
	uint		magic ;
	int		fd ;
} ;


#if	(! defined(UDP_MASTER)) || (UDP_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int udp_open(UDP *,SYSDIALER_ARGS *,cchar *,cchar *,cchar **) ;
extern int udp_reade(UDP *,char *,int,int,int) ;
extern int udp_recve(UDP *,char *,int,int,int,int) ;
extern int udp_recvfrome(UDP *,char *,int,int,void *,int *,int,int) ;
extern int udp_recvmsge(UDP *,struct msghdr *,int,int,int) ;
extern int udp_write(UDP *,const char *,int) ;
extern int udp_send(UDP *,const char *,int,int) ;
extern int udp_sendto(UDP *,const char *,int,int,void *,int) ;
extern int udp_sendmsg(UDP *,struct msghdr *,int) ;
extern int udp_shutdown(UDP *,int) ;
extern int udp_close(UDP *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UDP_MASTER */

#endif /* UDP_INCLUDE */


