/* connection */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CONNECTION_INCLUDE
#define	CONNECTION_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>

#include	<vecstr.h>
#include	<sockaddress.h>
#include	<localmisc.h>


/* local object defines */

#define	CONNECTION		struct connection_head
#define	CONNECTION_FL		struct connection_flags
#define	CONNECTION_PEERNAMELEN	MAX(MAXPATHLEN,MAXHOSTNAMELEN)


struct connection_flags {
	uint		inet:1 ;
	uint		trans:1 ;
	uint		sa:1 ;
	uint		addr:1 ;
	uint		domainname:1 ;	/* allocated */
} ;

struct connection_head {
	const char	*domainname ;	/* local domain name */
	const char	*peername ;	/* dynamically allocated */
	const char	*pr ;		/* dynamically allocated */
	CONNECTION_FL	f ;
	SOCKADDRESS	sa ;
	struct in_addr	netipaddr ;
	int		s ;
} ;


#if	(! defined(CONNECTION_MASTER)) || (CONNECTION_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int connection_start(CONNECTION *,const char *) ;
extern int connection_sockpeername(CONNECTION *,char *,int) ;
extern int connection_socksrcname(CONNECTION *,char *,int) ;
extern int connection_peername(CONNECTION *,SOCKADDRESS *,int,char *) ;
extern int connection_mknames(CONNECTION *,vecstr *) ;
extern int connection_finish(CONNECTION *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CONNECTION_MASTER */

#endif /* CONNECTION_INCLUDE */


