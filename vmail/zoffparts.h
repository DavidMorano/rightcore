/* zoffparts */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	ZOFFPATHS_INCLUDE
#define	ZOFFPARTS_INCLUDE	1


/* revision history:

	= 1995-08-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1995 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 
        This two small subroutine manipulate zone-offsets for use in time
        strings.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	ZOFFPARTS	struct zoffparts


struct zoffparts {
	uint	hours ;
	uint	mins ;
	int	zoff ;		/* value */
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	zoffparts_set(ZOFFPARTS *,int) ;
extern int	zoffparts_get(ZOFFPARTS *,int *) ;
extern int	zoffparts_mkstr(ZOFFPARTS *,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* ZOFFPARTS_INCLUDE */


