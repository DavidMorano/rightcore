/* usd */


#ifndef	USD_INCLUDE
#define	USD_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	USD_MAGIC	31415926
#define	USD		struct usd_head
#define	USD_FL		struct usd_flags


struct usd_flags {
	uint		log:1 ;
} ;

struct usd_head {
	uint		magic ;
	LOGFILE		lh ;
	USD_FL		f ;
	int		tlen ;
	int		fd ;
} ;


#if	(! defined(USD_MASTER)) || (USD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int usd_open(USD *,SYSDIALER_ARGS *,
		const char *,const char *,const char **) ;
extern int usd_reade(USD *,char *,int,int,int) ;
extern int usd_recve(USD *,char *,int,int,int,int) ;
extern int usd_recvfrome(USD *,char *,int,int,void *,int *,int,int) ;
extern int usd_recvmsge(USD *,struct msghdr *,int,int,int) ;
extern int usd_write(USD *,const char *,int) ;
extern int usd_send(USD *,const char *,int,int) ;
extern int usd_sendto(USD *,const char *,int,int,void *,int) ;
extern int usd_sendmsg(USD *,struct msghdr *,int) ;
extern int usd_shutdown(USD *,int) ;
extern int usd_close(USD *) ;

#ifdef	__cplusplus
}
#endif

#endif /* USD_MASTER */

#endif /* USD_INCLUDE */


