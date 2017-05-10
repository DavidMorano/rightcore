/* gethe */

/* get protocol entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GETHE_INCLUDE
#define	GETHE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netinet/in.h>
#include	<netdb.h>


#if	(! defined(GETHE_MASTER)) || (GETHE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int gethe_begin(int) ;
extern int gethe_ent(struct hostent *,char *,int) ;
extern int gethe_end() ;
extern int gethe_name(struct hostent *,char *,int,const char *) ;
extern int gethe_addr(struct hostent *,char *,int,in_addr_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETHE_MASTER */

#endif /* GETHE_INCLUDE */


