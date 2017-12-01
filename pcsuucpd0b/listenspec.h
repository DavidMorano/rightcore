/* listenspec */

/* this object holds a "listen" specification */
/* last modified %G% version %I% */


/* revision history:

	= 2001-03-23, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

#ifndef	LISTENSPEC_INCLUDE
#define	LISTENSPEC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sockaddress.h>


#define	LISTENSPEC_MAGIC	0x57245332
#define	LISTENSPEC		struct listenspec_head
#define	LISTENSPEC_FL		struct listenspec_flags
#define	LISTENSPEC_INFO		struct listenspec_i
#define	LISTENSPEC_PASS		struct listenspec_pass
#define	LISTENSPEC_USS		struct listenspec_uss
#define	LISTENSPEC_TCP		struct listenspec_tcp

#define	LISTENSPEC_MREUSE	(1<<0)	/* reuse address */
#define	LISTENSPEC_MACTIVE	(1<<0)	/* is active */
#define	LISTENSPEC_MDELPEND	(1<<1)	/* delete is pending */
#define	LISTENSPEC_MBROKEN	(1<<2)	/* activate failed (broken) */

#define	LISTENSPEC_TYPELEN	MAXNAMELEN
#define	LISTENSPEC_ADDRLEN	(MAXPATHLEN + 20)


struct listenspec_i {
	int		state ;
	char		type[LISTENSPEC_TYPELEN + 1] ;
	char		addr[LISTENSPEC_ADDRLEN + 1] ;
} ;

struct listenspec_pass {
	const char	*fname ;	/* dynamic allocation */
	int		mode ;
} ;

struct listenspec_uss {
	const char	*fname ;	/* dynamic allocation */
	int		mode ;
	SOCKADDRESS	sa ;
} ;

struct listenspec_tcp {
	void		*a ;		/* dynamic buffer allocation */
	const char	*af ;
	const char	*host ;
	const char	*port ;
	SOCKADDRESS	sa ;
} ;

struct listenspec_flags {
	uint		active:1 ;	/* actively listening */
	uint		delete:1 ;	/* marked for deletion */
	uint		broken:1 ;	/* activate attempt failed */
	uint		reuse:1 ;	/* re-use address */
} ;

struct listenspec_head {
	uint		magic ;
	void		*info ;		/* particular listener information */
	const char	*prlocal ;
	LISTENSPEC_FL	f ;
	int		ltype ;		/* "listen" type */
	int		fd ;
	int		rs_error ;
} ;


#if	(! defined(LISTENSPEC_MASTER)) || (LISTENSPEC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int listenspec_start(LISTENSPEC *,int,cchar **) ;
extern int listenspec_issame(LISTENSPEC *,LISTENSPEC *) ;
extern int listenspec_active(LISTENSPEC *,int,int) ;
extern int listenspec_isactive(LISTENSPEC *) ;
extern int listenspec_delset(LISTENSPEC *,int) ;
extern int listenspec_delmarked(LISTENSPEC *) ;
extern int listenspec_getfd(LISTENSPEC *) ;
extern int listenspec_gettype(LISTENSPEC *) ;
extern int listenspec_accept(LISTENSPEC *,void *,int *,int) ;
extern int listenspec_info(LISTENSPEC *,LISTENSPEC_INFO *) ;
extern int listenspec_clear(LISTENSPEC *) ;
extern int listenspec_geterr(LISTENSPEC *,int *) ;
extern int listenspec_finish(LISTENSPEC *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LISTENSPEC_MASTER */

#endif /* LISTENSPEC_INCLUDE */


