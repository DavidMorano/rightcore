/* ussnls */


#ifndef	USSNLS_INCLUDE
#define	USSNLS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	USSNLS_MAGIC		31415926
#define	USSNLS			struct ussnls_head
#define	USSNLS_FL		struct ussnls_flags


struct ussnls_flags {
	uint		log:1 ;

} ;

struct ussnls_head {
	uint		magic ;
	LOGFILE		lh ;
	USSNLS_FL	f, open ;
	int		tlen ;
	int		fd ;
} ;


#if	(! defined(USSNLS_MASTER)) || (USSNLS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ussnls_open(USSNLS *,SYSDIALER_ARGS *,
		const char *,const char *,const char **) ;
extern int ussnls_reade(USSNLS *,char *,int,int,int) ;
extern int ussnls_recve(USSNLS *,char *,int,int,int,int) ;
extern int ussnls_recvfrome(USSNLS *,char *,int,int,void *,int *,int,int) ;
extern int ussnls_recvmsge(USSNLS *,struct msghdr *,int,int,int) ;
extern int ussnls_write(USSNLS *,const char *,int) ;
extern int ussnls_send(USSNLS *,const char *,int,int) ;
extern int ussnls_sendto(USSNLS *,const char *,int,int,void *,int) ;
extern int ussnls_sendmsg(USSNLS *,struct msghdr *,int) ;
extern int ussnls_shutdown(USSNLS *,int) ;
extern int ussnls_close(USSNLS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* USSNLS_MASTER */

#endif /* USSNLS_INCLUDE */


