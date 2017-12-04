/* hostaddr */


/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

#ifndef	HOSTADDR_INCLUDE
#define	HOSTADDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>

#include	<vsystem.h>


#define	HOSTADDR_MAGIC		0x73625196
#define	HOSTADDR		struct hostaddr_head
#define	HOSTADDR_CUR		struct hostaddr_c


struct hostaddr_head {
	uint		magic ;
	struct addrinfo	*aip ;
	struct addrinfo	**resarr ;
	cchar		*ehostname ;
	int		n ;
} ;

struct hostaddr_c {
	int		i ;
} ;


#if	(! defined(HOSTADDR_MASTER)) || (HOSTADDR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int hostaddr_start(HOSTADDR *,cchar *,cchar *,struct addrinfo *) ;
extern int hostaddr_getcanonical(HOSTADDR *,cchar **) ;
extern int hostaddr_curbegin(HOSTADDR *,HOSTADDR_CUR *) ;
extern int hostaddr_curend(HOSTADDR *,HOSTADDR_CUR *) ;
extern int hostaddr_enum(HOSTADDR *,HOSTADDR_CUR *,struct addrinfo **) ;
extern int hostaddr_finish(HOSTADDR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* HOSTADDR_MASTER */

#endif /* HOSTADDR_INCLUDE */


