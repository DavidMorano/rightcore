/* strdcpy */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STRDCPY_INCLUDE
#define	STRDCPY_INCLUDE	1


#ifdef	__cplusplus
extern "C" {
#endif

extern char *strdcpy1(char *,int,cchar *) ;
extern char *strdcpy2(char *,int,cchar *,cchar *) ;
extern char *strdcpy3(char *,int,cchar *,cchar *,cchar *) ;
extern char *strdcpy4(char *,int,cchar *,cchar *,cchar *,
		cchar *) ;
extern char *strdcpy5(char *,int,cchar *,cchar *,cchar *,
		cchar *,cchar *) ;
extern char *strdcpy6(char *,int,cchar *,cchar *,cchar *,
		cchar *,cchar *,cchar *) ;

extern char *strdcpy1w(char *,int,cchar *,int) ;
extern char *strdcpy2w(char *,int,cchar *,cchar *,int) ;
extern char *strdcpy3w(char *,int,cchar *,cchar *,cchar *,
		int) ;
extern char *strdcpy4w(char *,int,cchar *,cchar *,cchar *,
		cchar *,int) ;
extern char *strdcpy5w(char *,int,cchar *,cchar *,cchar *,
		cchar *,cchar *,int) ;
extern char *strdcpy6w(char *,int,cchar *,cchar *,cchar *,
		cchar *,cchar *,cchar *,int) ;

extern char	*strdcpycompact(char *,int,cchar *,int) ;
extern char	*strdcpyopaque(char *,int,cchar *,int) ;
extern char	*strdcpyclean(char *,int,cchar *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRDCPY_INCLUDE	*/


