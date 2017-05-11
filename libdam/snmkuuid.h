/* snmkuuid */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SNMKUUID_INCLUDE
#define	SNMKUUID_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<mkuuid.h>
#include	<localmisc.h>


#if	(! defined(SNMKUUID_MASTER)) || (SNMKUUID_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int snmkuuid(char *,int,MKUUID *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SNMKUUID_MASTER */

#endif /* SNMKUUID_INCLUDE */


