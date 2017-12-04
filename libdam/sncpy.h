/* sncpy */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SNCPY_INCLUDE
#define	SNCPY_INCLUDE	1


#include	<localmisc.h>		/* for extra unsigned types */


#ifdef	__cplusplus
extern "C" {
#endif

extern int sncpy(char *,int,int,...) ;
extern int sncpy1(char *,int,cchar *) ;
extern int sncpy2(char *,int,cchar *,cchar *) ;
extern int sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern int sncpy5(char *,int,cchar *,cchar *,cchar *,cchar *,cchar *) ;
extern int sncpy6(char *,int,cchar *,cchar *,cchar *,cchar *,cchar *,cchar *) ;

extern int sncpy1w(char *,int,cchar *,int) ;
extern int sncpy2w(char *,int,cchar *,cchar *,int) ;
extern int sncpy3w(char *,int,cchar *,cchar *,cchar *,int) ;
extern int sncpy4w(char *,int,cchar *,cchar *,cchar *,cchar *,int) ;
extern int sncpy5w(char *,int,cchar *,cchar *,cchar *,cchar *,cchar *,int) ;
extern int sncpy6w(char *,int,cchar *,cchar *,cchar *,cchar *,cchar *,
		cchar *,int) ;

extern int sncpyarray(char *,int,cchar **,int) ;
extern int sncpyw(char *,cchar *,int) ;

extern int sncpylc(char *,int,cchar *) ;
extern int sncpyuc(char *,int,cchar *) ;
extern int sncpyfc(char *,int,cchar *) ;

extern int sncat1(char *,int,cchar *) ;
extern int sncat2(char *,int,cchar *,cchar *) ;

/* special specialized */
extern int snsds(char *,int,cchar *,cchar *) ;
extern int snscs(char *,int,cchar *,cchar *) ;
extern int snses(char *,int,cchar *,cchar *) ;
extern int snsd(char *,int,cchar *,uint) ;
extern int snsdd(char *,int,cchar *,uint) ;
extern int snddd(char *,int,uint,uint) ;
extern int snfsflags(char *,int,ulong) ;
extern int snopenflags(char *,int,int) ;
extern int snpollflags(char *,int,int) ;
extern int snxtilook(char *,int,int) ;
extern int sninetaddr(char *,int,int,const char *) ;
extern int snsigabbr(char *,int,uint) ;
extern int snabbr(char *,int,cchar *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SNCPY_INCLUDE	*/


