/* getne */

/* get protocol entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GETNE_INCLUDE
#define	GETNE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netdb.h>


#if	(! defined(GETNE_MASTER)) || (GETNE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int getne_begin(int) ;
extern int getne_ent(struct netent *,char *,int) ;
extern int getne_end() ;
extern int getne_name(struct netent *,char *,int,const char *) ;
extern int getne_addr(struct netent *,char *,int,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETNE_MASTER */

#endif /* GETNE_INCLUDE */


