/* uss */


#ifndef	USS_INCLUDE
#define	USS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<logfile.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	USS_MAGIC	31415926
#define	USS		struct uss_head
#define	USS_FL		struct uss_flags


struct uss_flags {
	uint		log:1 ;
} ;

struct uss_head {
	uint		magic ;
	LOGFILE		lh ;
	USS_FL		open ;
	int		tlen ;
	int		fd ;
} ;


#if	(! defined(USS_MASTER)) || (USS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uss_open(USS *,SYSDIALER_ARGS *,
		const char *,const char *,const char **) ;
extern int uss_reade(USS *,char *,int,int,int) ;
extern int uss_recve(USS *,char *,int,int,int,int) ;
extern int uss_recvfrome(USS *,char *,int,int,void *,int *,int,int) ;
extern int uss_recvmsge(USS *,struct msghdr *,int,int,int) ;
extern int uss_write(USS *,const char *,int) ;
extern int uss_send(USS *,const char *,int,int) ;
extern int uss_sendto(USS *,const char *,int,int,void *,int) ;
extern int uss_sendmsg(USS *,struct msghdr *,int) ;
extern int uss_shutdown(USS *,int) ;
extern int uss_close(USS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* USS_MASTER */

#endif /* USS_INCLUDE */


