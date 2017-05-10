/* splitaddr */

/* splitaddr mail management */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SPLITADDR_INCLUDE
#define	SPLITADDR_INCLUDE	1


#include	<sys/types.h>

#include	<vsystem.h>
#include	<vechand.h>
#include	<localmisc.h>


#define	SPLITADDR	struct splitaddr_head

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif


struct splitaddr_head {
	vechand		coms ;
	const char	*local ;
	cchar		*mailaddr ;
	int		nd ;
} ;


#if	(! defined(SPLITADDR_MASTER)) || (SPLITADDR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int splitaddr_start(SPLITADDR *,const char *) ;
extern int splitaddr_prematch(SPLITADDR *,SPLITADDR *) ;
extern int splitaddr_finish(SPLITADDR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SPLITADDR_MASTER */

#endif /* SPLITADDR_INCLUDE */


