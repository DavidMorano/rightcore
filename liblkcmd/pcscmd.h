/* pcs-cmd */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSCMD_INCLUDE
#define	PCSCMD_INCLUDE	1

#include	<envstandards.h>

#include	"pcsmain.h"
#include	"defs.h"		/* for PROGINFO */


#ifdef	__cplusplus
extern "C" {
#endif

extern int	pcscmd(PROGINFO *,cchar *) ;
extern int	pcscmd_svcname(PROGINFO *,int,cchar **) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSCMD_INCLUDE */


