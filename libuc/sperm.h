/* sperm */

/* stat-perm */


/* revision history:

	= 1998-04-05, David A­D­ Morano
	This module was adapted from assembly lanauge.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	We check for specified permissions on a file given an IDS object (for 
	the user to check permissions for) and the |stat(2)| of the file in 
	question.


******************************************************************************/


#ifndef	SPERM_INCLUDE
#define	SPERM_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>
#include	<ids.h>


#if	(! defined(SPERM_MASTER)) || (SPERM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sperm(IDS *,struct ustat *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SPERM_MASTER */

#endif /* SPERM_INCLUDE */


