/* sockaddress */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SOCKADDRESS_INCLUDE
#define	SOCKADDRESS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/un.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

#include	<localmisc.h>


#define	SOCKADDRESS		union sockaddress_head
#define	SOCKADDRESS_INET	struct sockaddress_inet6

#define	SOCKADDRESS_LEN		sizeof(union sockaddress_head)
#define	SOCKADDRESS_NAMELEN	sizeof(union sockaddress_head)


#ifndef	PF_INET4
#define	PF_INET4	PF_INET
#endif

#ifndef	PF_INET6
#define	PF_INET6	26	/* Solaris 8 but not early 2.5.1! */
#endif

#ifndef	AF_INET4
#define	AF_INET4	AF_INET
#endif

#ifndef	AF_INET6
#define	AF_INET6	26	/* Solaris 8 but not early 2.5.1! */
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

#ifndef	UCHAR
#define	UCHAR	unsigned char
#endif

#ifndef	USHORT
#define	USHORT	unsigned short
#endif

#ifndef	UINT
#define	UINT	unsigned int
#endif


/* all values should be in network byte order */

struct sockaddress_path {
	USHORT	naf ;			/* address space */
	char	path[MAXPATHLEN + 1] ;
} ;

struct sockaddress_inet4 {
	USHORT	naf ;			/* address space */
	USHORT	nport ;			/* port */
	UCHAR	a[4] ;			/* 32-bit address */
} ;

struct sockaddress_inet6 {
	USHORT	naf ;			/* address space */
	USHORT	nport ;			/* port */
	UINT	nflow ;			/* flow label */
	UCHAR	a[16] ;			/* 128-bit address */
	UINT	scope ;			/* scope crap */
	UCHAR	dummy[4] ;		/* implementation defined */
} ;

union sockaddress_head {
	struct sockaddr			a_unspec ;
	struct sockaddr_un		a_unix ;
	struct sockaddr_in		a_in ;
	struct sockaddr_in6		a_in6 ;
	struct sockaddress_inet4	a_inet4 ;
	struct sockaddress_inet6	a_inet6 ;
	struct sockaddress_path		a_path ;
	unsigned char			str[32] ; /* handle IPV6 for future */
} ;


typedef	SOCKADDRESS		sockaddress ;
typedef	SOCKADDRESS_INET	sockaddress_inet ;


#if	(! defined(SOCKADDRESS_MASTER)) || (SOCKADDRESS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sockaddress_start(SOCKADDRESS *,int,const void *,
			int,int) ;
extern int	sockaddress_startaddr(SOCKADDRESS *,int,const void *,
			int,int,int) ;
extern int	sockaddress_getlen(SOCKADDRESS *) ;
extern int	sockaddress_getaddrlen(SOCKADDRESS *) ;
extern int	sockaddress_gethex(SOCKADDRESS *,char *,int) ;
extern int	sockaddress_getaf(SOCKADDRESS *) ;
extern int	sockaddress_getport(SOCKADDRESS *) ;
extern int	sockaddress_getflow(SOCKADDRESS *,uint *) ;
extern int	sockaddress_getaddr(SOCKADDRESS *,void *,int) ;
extern int	sockaddress_getscope(SOCKADDRESS *,uint *) ;
extern int	sockaddress_getextra(SOCKADDRESS *,uint *) ;
extern int	sockaddress_putaf(SOCKADDRESS *,int) ;
extern int	sockaddress_putport(SOCKADDRESS *,int) ;
extern int	sockaddress_putaddr(SOCKADDRESS *,const void *) ;
extern int	sockaddress_finish(SOCKADDRESS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SOCKADDRESS_MASTER */

#endif /* SOCKADDRESS_INCLUDE */


