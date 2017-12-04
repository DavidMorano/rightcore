/* hostinfo */

/* last modified %G% version %I% */


/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

#ifndef	HOSTINFO_INCLUDE
#define	HOSTINFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>


/* miscellaneous stuff (that everyone should enjoy) */

#ifndef	PF_INET4
#ifdef	PF_INET
#define	PF_INET4	PF_INET
#else
#define	PF_INET4	2
#endif
#endif

#ifndef	AF_INET4
#ifdef	AF_INET
#define	AF_INET4	AF_INET
#else
#define	AF_INET4	2
#endif
#endif

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(in_addr_t)
#endif

#ifndef	INET6ADDRLEN
#define	INET6ADDRLEN	16
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	MAX(INET4ADDRLEN,INET6ADDRLEN)
#endif

#define	HOSTINFO_MAGIC		0x73625196
#define	HOSTINFO		struct hostinfo_head
#define	HOSTINFO_FL		struct hostinfo_flags
#define	HOSTINFO_ADDR		struct hostinfo_addr
#define	HOSTINFO_ARGS		struct hostinfo_args
#define	HOSTINFO_CUR		struct hostinfo_c


struct hostinfo_flags {
	uint		inet4:1 ;
	uint		inet6:1 ;
	uint		addr:1 ;	/* was given an address */
} ;

struct hostinfo_addr {
	const char	*addr ;
	int		addrlen ;
	int		af ;
} ;

struct hostinfo_args {
	const char	*hostname ;	/* might be allocated */
	uint		af ;		/* caller-supplied argument */
	int		hostnamelen ;
	int		f_alloc ;
} ;

struct hostinfo_head {
	uint		magic ;
	HOSTINFO_FL	init, f ;
	HOSTINFO_ARGS	arg ;
	HOSTINFO_ADDR	addr ;
	VECOBJ		names ;
	VECOBJ		addrs ;
	const char	*domainname ;	/* dynamically allocated */
	char		ehostname[MAXHOSTNAMELEN + 1] ;
	char		chostname[MAXHOSTNAMELEN + 1] ;
} ;

struct hostinfo_c {
	int		i ;
} ;


#if	(! defined(HOSTINFO_MASTER)) || (HOSTINFO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int hostinfo_start(HOSTINFO *,int,cchar *) ;
extern int hostinfo_geteffective(HOSTINFO *,const char **) ;
extern int hostinfo_getcanonical(HOSTINFO *,const char **) ;
extern int hostinfo_curbegin(HOSTINFO *,HOSTINFO_CUR *) ;
extern int hostinfo_curend(HOSTINFO *,HOSTINFO_CUR *) ;
extern int hostinfo_enumname(HOSTINFO *,HOSTINFO_CUR *,const char **) ;
extern int hostinfo_enumaddr(HOSTINFO *,HOSTINFO_CUR *,const uchar **) ;
extern int hostinfo_finish(HOSTINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* HOSTINFO_MASTER */

#endif /* HOSTINFO_INCLUDE */


