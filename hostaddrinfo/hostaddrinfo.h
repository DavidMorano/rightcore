/* hostinfo */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	HOSTINFO_INCLUDE
#define	HOSTINFO_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>

#include	<vsystem.h>


/* local object defines */

#define	HOSTINFO		struct hostinfo_head
#define	HOSTINFO_CURSOR		struct hostinfo_c


struct hostinfo_head {
	unsigned long	magic ;
	struct addrinfo	*aip ;
} ;

struct hostinfo_c {
	struct addrinfo	*aip ;
} ;


#if	(! defined(HOSTINFO_MASTER)) || (HOSTINFO_MASTER == 0)

extern int hostinfo_init(HOSTINFO *,cchar *,cchar *,cchar *,
		struct addrinfo *) ;
extern int hostinfo_getofficial(HOSTINFO *,char **) ;
extern int hostinfo_getcanonical(HOSTINFO *,char **) ;
extern int hostinfo_curbegin(HOSTINFO *,HOSTINFO_CURSOR *) ;
extern int hostinfo_curend(HOSTINFO *,HOSTINFO_CURSOR *) ;
extern int hostinfo_enumname(HOSTINFO *,HOSTINFO_CURSOR *,char **) ;
extern int hostinfo_enumaddr(HOSTINFO *,HOSTINFO_CURSOR *,char **) ;
extern int hostinfo_enum(HOSTINFO *,HOSTINFO_CURSOR *,
		struct addrinfo **) ;
extern int hostinfo_free(HOSTINFO *) ;

#endif /* HOSTINFO_MASTER */

#endif /* HOSTINFO_INCLUDE */


