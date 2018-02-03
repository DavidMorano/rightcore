/* protoent */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	PROTOENT_INCLUDE
#define	PROTOENT_INCLUDE		1


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

#define	PROTOENT	struct protoent
#define	PROTOENT_CUR	struct protoent_c


struct protoent_c {
	int		i ;
} ;


typedef struct protoent_c	protoent_cur ;


#if	(! defined(PROTOENT_MASTER)) || (PROTOENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uc_getprotobyname(int,PROTOENT *,char *,int) ;
extern int uc_getprotobynumber(cchar *,int,PROTOENT *,char *rbuf,int rlen) ;
extern int uc_getprotoent(PROTOENT *,char *,int ) ;
extern int uc_getprotobyname(cchar *,PROTOENT *,char *,int) ;
extern int uc_getprotobynumber(int,PROTOENT *,proto,pep,rbuf,rlen)

#ifdef	__cplusplus
}
#endif

#endif /* PROTOENT_MASTER */

#endif /* PROTOENT_INCLUDE */


