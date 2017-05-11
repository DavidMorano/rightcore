/* termcharset */

/* terminal-character-setter */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	TERMCHARSET_INCLUDE
#define	TERMCHARSET_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#if	(! defined(TERMCHARSET_MASTER)) || (TERMCHARSET_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int termcharset(char *,int,int,int,cchar *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TERMCHARSET_MASTER */

#endif /* TERMCHARSET_INCLUDE */


