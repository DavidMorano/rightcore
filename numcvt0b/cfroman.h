/* cfroman */
/* lang=C++11 */

/* convert from Roman Numeral to binary integer */


/* revision history:

	= 2017-08-15, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

#ifndef	CFROMAN_INCLUDE
#define	CFROMAN_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#if	(! defined(CFROMAN_MASTER)) || (CFROMAN_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int cfroman(cchar *,int,* int) ;

extern int cfromani(cchar *,int,* int) ;
extern int cfromanl(cchar *,int,* long) ;
extern int cfromanll(cchar *,int,* longlong) ;

extern int cfromanui(cchar *,int,* uint) ;
extern int cfromanul(cchar *,int,* ulong) ;
extern int cfromanull(cchar *,int,* ulonglong) ;

#ifdef	__cplusplus
}
#endif

#endif /* CFROMAN_MASTER */

#endif /* CFROMAN_INCLUDE */


