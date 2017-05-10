/* getus */

/* get protocol entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GETUS_INCLUDE
#define	GETUS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netdb.h>


#if	(! defined(GETUS_MASTER)) || (GETUS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int getus_begin() ;
extern int getus_ent(char *,int) ;
extern int getus_end() ;

#ifdef	__cplusplus
}
#endif

#endif /* GETUS_MASTER */

#endif /* GETUS_INCLUDE */


