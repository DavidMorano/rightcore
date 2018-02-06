/* getproviderid */

/* get a provider ID */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GETPROVIDERID_INCLUDE
#define	GETPROVIDERID_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


#define	PROVIDERID_UNKNOWN	0
#define	PROVIDERID_SUN		1
#define	PROVIDERID_COMPAQ	2
#define	PROVIDERID_SGI		3
#define	PROVIDERID_DELL		4
#define	PROVIDERID_HP		5
#define	PROVIDERID_OVERLAST	6


#if	(! defined(GETPROVIDERID_MASTER)) || (GETPROVIDERID_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int getproviderid(const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETPROVIDERID_MASTER */

#endif /* GETPROVIDERID_INCLUDE */


