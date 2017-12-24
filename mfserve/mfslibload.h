/* mfslibload */

/* shared-object library-load module */
/* last modified %G% version %I% */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to insert into MFSERVE.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */


#ifndef	MFSLIBLOAD_INCLUDE
#define	MFSLIBLOAD_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"sreq.h"


#if	(! defined(MFSLIBLOAD_MASTER)) || (MFSLIBLOAD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mfslibload_begin(PROGINFO *,SREQ *) ;
extern int mfslibload_end(PROGINFO *,SREQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSLIBLOAD_MASTER */


#endif /* MFSLIBLOAD_INCLUDE */


