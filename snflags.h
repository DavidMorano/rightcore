/* snflags */

/* make string version of some flags */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Ths object is used in the creation of flags strings.


******************************************************************************/


#ifndef	SNFLAGS_INCLUDE
#define	SNFLAGS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */


#define	SNFLAGS		struct snflags_head


struct snflags_head {
	char		*bp ;
	int		c ;
	int		bl ;
	int		bi ;
} ;


#if	(! defined(SNFLAGS_MASTER)) || (SNFLAGS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int snflags_start(SNFLAGS *,char *,int) ;
extern int snflags_addstr(SNFLAGS *,cchar *) ;
extern int snflags_finish(SNFLAGS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SNFLAGS_MASTER */

#endif /* SNFLAGS_INCLUDE */


