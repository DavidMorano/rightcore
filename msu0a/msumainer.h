/* msumainer */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MSUMAINER_INCLUDE
#define	MSUMAINER_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<lfm.h>
#include	"msumain.h"
#include	"defs.h"		/* for PROGINFO */


#ifdef	__cplusplus
extern "C" {
#endif

extern int	msumainlockprint(PROGINFO *,cchar *,LFM_CHECK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MSUMAINER_INCLUDE */


