/* sntmtime */

/* create a counted string from date-time information */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SNTMTIME_INCLUDE
#define	SNTMTIME_INCLUDE	1


#include	<envstandards.h>
#include	<tmtime.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int sntmtime(char *,int,TMTIME *,const char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SNTMTIME_INCLUDE */


