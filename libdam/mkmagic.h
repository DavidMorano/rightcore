/* mkmagic */
/* lang=C99 */


/* revision history:

	= 2017-07-17, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

#ifndef	MKMAGIC_INCLUDE
#define	MKMAGIC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<localmisc.h>		/* extra types */


#if	(! defined(MKMAGIC_MASTER)) || (MKMAGIC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mkmagic(char *,int,cchar *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MKMAGIC_MASTER */

#endif /* MKMAGIC_INCLUDE */


