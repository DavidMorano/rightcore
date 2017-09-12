/* partitionai */


/* revision history:

	= 2001-10-04, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */


#ifndef	PARTITIONAI_INCLUDE
#define	PARTITIONAI_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#if	(! defined(PARTITIONAI_MASTER)) || (PARTITIONAI_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	partitionai(int *,int,int (*)(int,int),int) ;

#ifdef	__cplusplus
}
#endif

#endif /* PARTITIONAI_MASTER */

#endif /* PARTITIONAI_INCLUDE */


