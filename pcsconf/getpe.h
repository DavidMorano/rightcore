/* getpe */

/* get protocol entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GETPE_INCLUDE
#define	GETPE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netdb.h>


#if	(! defined(GETPE_MASTER)) || (GETPE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int getpe_begin(int) ;
extern int getpe_ent(struct protoent *,char *,int) ;
extern int getpe_end() ;
extern int getpe_name(struct protoent *,char *,int,const char *) ;
extern int getpe_num(struct protoent *,char *,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETPE_MASTER */

#endif /* GETPE_INCLUDE */


