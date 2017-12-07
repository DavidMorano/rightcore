/* mfserve */

/* support for loadable modules for MFSERVE */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSERVE_INCLUDE
#define	MFSERVE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* object defines */

#define	MFSERVE_MOD		struct mfserve_mod


struct mfserve_mod {
	cchar		*objname ;
	int		objsize ;
	int		endmark ;
} ;


#endif /* MFSERVE_INCLUDE */


