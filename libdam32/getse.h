/* getse */

/* get service entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GETSE_INCLUDE
#define	GETSE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netdb.h>


#if	(! defined(GETSE_MASTER)) || (GETSE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int getse_begin(int) ;
extern int getse_ent(struct servent *,char *,int) ;
extern int getse_end() ;
extern int getse_name(struct servent *,char *,int,const char *,const char *) ;
extern int getse_port(struct servent *,char *,int,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETSE_MASTER */

#endif /* GETSE_INCLUDE */


