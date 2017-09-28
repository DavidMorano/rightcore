/* ctroman */
/* lang=C++11 */

/* convert to Number Words */


/* revision history:

	= 2017-08-15, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

#ifndef	CTROMAN_INCLUDE
#define	CTROMAN_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#if	(! defined(CTROMAN_MASTER)) || (CTROMAN_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ctroman(char *,int,int) ;

extern int ctromani(char *,int,int) ;
extern int ctromanl(char *,int,long) ;
extern int ctromanll(char *,int,longlong) ;

extern int ctromanui(char *,int,uint) ;
extern int ctromanul(char *,int,ulong) ;
extern int ctromanull(char *,int,ulonglong) ;

#ifdef	__cplusplus
}
#endif

#endif /* CTROMAN_MASTER */

#endif /* CTROMAN_INCLUDE */


