/* mfs-cmd */

/* last modified %G% version %I% */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2004,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSCMD_INCLUDE
#define	MFSCMD_INCLUDE	1

#include	<envstandards.h>

#include	"mfsmain.h"
#include	"defs.h"		/* for PROGINFO */


#ifdef	__cplusplus
extern "C" {
#endif

extern int	mfscmd(PROGINFO *,cchar *) ;
extern int	mfscmd_svcname(PROGINFO *,int,cchar **) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSCMD_INCLUDE */


