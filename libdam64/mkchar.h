/* mkchar INCLUDE */

/* make a character out of an integer */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MKCHAR_INCLUDE
#define	MKCHAR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


#if	(! defined(MKCHAR_MASTER)) || (MKCHAR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mkchar(int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MKCHAR_MASTER */

#endif /* MKCHAR_INCLUDE */


