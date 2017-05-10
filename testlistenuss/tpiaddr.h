/* tpiaddr */



#ifndef	TPIADDR_INCLUDE
#define	TPIADDR_INCLUDE	1



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/un.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>



#define	TPIADDR		union tpiaddr_head

#ifndef	PF_INET6
#define	PF_INET6	26	/* Solaris 8 but not early 2.5.1 ! */
#endif

#ifndef	AF_INET6
#define	AF_INET6	26	/* Solaris 8 but not early 2.5.1 ! */
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

struct tpiaddr_path {
	USHORT	af ;
	char	path[MAXPATHLEN + 1] ;
} ;

struct tpiaddr_inet6 {
	USHORT	af ;
	USHORT	port ;
	UINT	flow ;
	UCHAR	aa[16] ;
} ;

union tpiaddr_head {
	struct sockaddr			a_unspec ;
	struct sockaddr_un		a_unix ;
	struct sockaddr_in		a_inet ;
	struct tpiaddr_inet6		al_inet6 ; /* LOCAL ! */
	struct tpiaddr_path		a_path ;
	unsigned char			str[24] ; /* handle IPV6 for future */
} ;



typedef union tpiaddr_head	tpiaddr ;



#ifndef	TPIADDR_MASTER

extern int	tpiaddr_init(TPIADDR *,int,void *,int, ...) ;
extern int	tpiaddr_initaddr(TPIADDR *,int,void *,int,int, ...) ;
extern int	tpiaddr_getaf(TPIADDR *,int *) ;
extern int	tpiaddr_getport(TPIADDR *,int *) ;
extern int	tpiaddr_getaddr(TPIADDR *,void *,int) ;
extern int	tpiaddr_putaddr(TPIADDR *,void *) ;
extern int	tpiaddr_free(TPIADDR *) ;
extern int	tpiaddr_gethex(TPIADDR *,char *,int) ;
extern int	tpiaddr_getlen(TPIADDR *) ;

extern TPIADDR	obj_tpiaddr(int,int,void *, ...) ;
extern TPIADDR	*new_tpiaddr(int,int,void *, ...) ;
extern void	free_tpiaddr(TPIADDR *) ;

#endif /* TPIADDR_MASTER */


#endif /* TPIADDR_INCLUDE */



