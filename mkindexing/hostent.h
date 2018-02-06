/* hostent */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	HOSTENT_INCLUDE
#define	HOSTENT_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>		/* extra typedefs */


/* local object defines */

#define	HOSTENT		struct hostent
#define	HOSTENT_CUR	struct hostent_c


struct hostent_c {
	int		i ;
} ;


typedef struct hostent_c	hostent_cur ;


#if	(! defined(HOSTENT_MASTER)) || (HOSTENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int hostent_getofficial(HOSTENT *,const char **) ;
extern int hostent_getcanonical(HOSTENT *,const char **) ;
extern int hostent_getaf(HOSTENT *) ;
extern int hostent_getalen(HOSTENT *) ;
extern int hostent_curbegin(HOSTENT *,HOSTENT_CUR *) ;
extern int hostent_curend(HOSTENT *,HOSTENT_CUR *) ;
extern int hostent_enumname(HOSTENT *,HOSTENT_CUR *,const char **) ;
extern int hostent_enumaddr(HOSTENT *,HOSTENT_CUR *,const uchar **) ;
extern int hostent_size(HOSTENT *) ;
extern int hostent_load(HOSTENT *,char *,int,HOSTENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* HOSTENT_MASTER */

#endif /* HOSTENT_INCLUDE */


