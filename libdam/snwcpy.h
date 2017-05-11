/* snwcpy */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SNWCPY_INCLUDE
#define	SNWCPY_INCLUDE	1


#include	<sys/types.h>		/* for system types */
#include	<stddef.h>		/* for 'wchar_t' */
#include	<localmisc.h>		/* for extra types */


#ifdef	__cplusplus
extern "C" {
#endif

extern int snwcpy(char *,int,const char *,int) ;
extern int snwcpylc(char *,int,const char *,int) ;
extern int snwcpyuc(char *,int,const char *,int) ;
extern int snwcpyfc(char *,int,const char *,int) ;
extern int snwcpyhyphen(char *,int,const char *,int) ;
extern int snwcpyshrink(char *,int,const char *,int) ;
extern int snwcpylatin(char *,int,const char *,int) ;
extern int snwcpyopaque(char *,int,const char *,int) ;
extern int snwcpycompact(char *,int,const char *,int) ;
extern int snwcpyclean(char *,int,const char *,int) ;
extern int snwcpywidehdr(char *,int,const wchar_t *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SNWCPY_INCLUDE	*/


