/* cm */
/* Connection-Manager */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CM_INCLUDE
#define	CM_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<unistd.h>

#include	"systems.h"
#include	"sysdialer.h"


#define	CM_MAGIC	31815926
#define	CM		struct cm_head
#define	CM_ARGS		struct cm_args
#define	CM_INFO		struct cm_info


struct cm_args {
	const char	*searchname ;
	const char	*pr ;		/* program-root */
	const char	*prn ;		/* program root-name */
	const char	*nodename ;
	const char	*domainname ;
	const char	*hostname ;
	const char	*username ;
	SYSTEMS		*sp ;
	SYSDIALER	*dp ;
	int		timeout ;	/* connection timeout */
	int		options ;
} ;

struct cm_info {
	int		itype ;
	int		dflags ;
	char		dname[MAXNAMELEN + 1] ;	/* "sysdialer name" */
} ;

struct cm_head {
	uint		magic ;
	void		*addr ;		/* remote address */
	void		*dobj ;		/* individual sysdialer object */
	const char	*dname ;	/* sysdialer name */
	CM_ARGS		*ap ;
	SYSDIALER_CALLS	c ;
	int		dsize ;		/* individual sysdialer object size */
	int		dflags ;	/* sysdialer flags */
	int		itype ;
	int		fd ;
	int		addrlen ;
} ;


#if	(! defined(CM_MASTER)) || (CM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int cm_open(CM *,CM_ARGS *,const char *,const char *,const char **) ;
extern int cm_info(CM *,CM_INFO *) ;
extern int cm_reade(CM *,char *,int,int,int) ;
extern int cm_recve(CM *,char *,int,int,int,int) ;
extern int cm_recvfrome(CM *,char *,int,int,void *,int *, int,int) ;
extern int cm_recvmsge(CM *,struct msghdr *,int,int,int) ;
extern int cm_write(CM *,cchar *,int) ;
extern int cm_send(CM *,cchar *,int,int) ;
extern int cm_sendto(CM *,cchar *,int,int,void *,int) ;
extern int cm_sendmsg(CM *,struct msghdr *,int) ;
extern int cm_poll(CM *,int) ;
extern int cm_localname(CM *,char *,int) ;
extern int cm_peername(CM *,char *,int) ;
extern int cm_shutdown(CM *,int) ;
extern int cm_close(CM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CM_MASTER */

#endif /* CM_INCLUDE */


