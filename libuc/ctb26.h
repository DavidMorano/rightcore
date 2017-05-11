/* ctb26 */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CTB26_INCLUDE
#define	CTB26_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int ctb26(char *,int,int,int,int) ;

extern int ctb26i(char *,int,int,int,int) ;
extern int ctb26l(char *,int,int,int,long) ;
extern int ctb26ll(char *,int,int,int,LONG) ;

extern int ctb26ui(char *,int,int,int,uint) ;
extern int ctb26ul(char *,int,int,int,ulong) ;
extern int ctb26ull(char *,int,int,int,ULONG) ;

#ifdef	__cplusplus
}
#endif

#endif /* CTB26_INCLUDE	*/


