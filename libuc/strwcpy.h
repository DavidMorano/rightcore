/* strwcpy */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STRWCPY_INCLUDE
#define	STRWCPY_INCLUDE	1


#ifdef	__cplusplus
extern "C" {
#endif

extern char	* strwcpy (char *,const char *,int) ;
extern char	* strwcpylc (char *,const char *,int) ;
extern char	* strwcpyuc (char *,const char *,int) ;
extern char	* strwcpyfc (char *,const char *,int) ;
extern char	* strwcpyblanks (char *,int) ;
extern char	* strwcpyopaque(char *,const char *,int) ;
extern char	* strwcpycompact(char *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRWCPY_INCLUDE	*/


