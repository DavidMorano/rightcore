/* inetaddr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	INETADDR_INCLUDE
#define	INETADDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>


/* object defines */

#define	INETADDR	union inetaddr_head


union inetaddr_head {
	struct in_addr	a ;
	char		straddr[sizeof(struct in_addr)] ;
} ;


typedef union inetaddr_head	inetaddr ;


#if	(! defined(INETADDR_MASTER)) || (INETADDR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int inetaddr_start(INETADDR *,const void *) ;
extern int inetaddr_startstr(INETADDR *,const char *,int) ;
extern int inetaddr_startdot(INETADDR *,const char *,int) ;
extern int inetaddr_gethexaddr(INETADDR *,char *,int) ;
extern int inetaddr_getdotaddr(INETADDR *,char *,int) ;
extern int inetaddr_finish(INETADDR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* INETADDR_MASTER */

#endif /* INETADDR_INCLUDE */


