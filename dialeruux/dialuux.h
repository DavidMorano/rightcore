/* dialuux */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DIALUUX_INCLUDE
#define	DIALUUX_INCLUDE		1


#include	<vsystem.h>	/* function-modifier definitions */


#define	DIALUUX_OQUEUE		FM_QUEUE
#define	DIALUUX_ONOREPORT	FM_NOREPORT


#ifdef	__cplusplus
extern "C" {
#endif

extern int dialuux(const char *,const char *,const char *,
		const char **,const char *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* DIALUUX_INCLUDE */


