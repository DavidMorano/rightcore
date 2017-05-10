/* ussmux */


#ifndef	USSMUX_INCLUDE
#define	USSMUX_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<logfile.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	USSMUX_MAGIC		31415926
#define	USSMUX			struct ussmux_head
#define	USSMUX_FL		struct ussmux_flags


struct ussmux_flags {
	uint		log:1 ;

} ;

struct ussmux_head {
	uint		magic ;
	LOGFILE		lh ;
	USSMUX_FL	f, open ;
	int		tlen ;
	int		fd ;
} ;


#if	(! defined(USSMUX_MASTER)) || (USSMUX_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ussmux_open(USSMUX *,SYSDIALER_ARGS *,
		const char *,const char *,const char **) ;
extern int ussmux_reade(USSMUX *,char *,int,int,int) ;
extern int ussmux_recve(USSMUX *,char *,int,int,int,int) ;
extern int ussmux_recvfrome(USSMUX *,char *,int,int,void *,int *,int,int) ;
extern int ussmux_recvmsge(USSMUX *,struct msghdr *,int,int,int) ;
extern int ussmux_write(USSMUX *,const char *,int) ;
extern int ussmux_send(USSMUX *,const char *,int,int) ;
extern int ussmux_sendto(USSMUX *,const char *,int,int,void *,int) ;
extern int ussmux_sendmsg(USSMUX *,struct msghdr *,int) ;
extern int ussmux_shutdown(USSMUX *,int) ;
extern int ussmux_close(USSMUX *) ;

#ifdef	__cplusplus
}
#endif

#endif /* USSMUX_MASTER */

#endif /* USSMUX_INCLUDE */


