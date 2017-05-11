/* mknpath */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MKNPATH_INCLUDE
#define	MKNPATH_INCLUDE	1


#include	<localmisc.h>		/* for extra unsigned types */


#ifdef	__cplusplus
extern "C" {
#endif

extern int mknpath1(char *,int,const char *) ;
extern int mknpath2(char *,int,const char *,const char *) ;
extern int mknpath3(char *,int,const char *,const char *,const char *) ;
extern int mknpath4(char *,int,const char *,const char *,const char *,
		const char *) ;
extern int mknpath5(char *,int,const char *,const char *,const char *,
		const char *,const char *) ;
extern int mknpath6(char *,int,const char *,const char *,const char *,
		const char *,const char *,const char *) ;

extern int mknpath1w(char *,int,const char *,int) ;
extern int mknpath2w(char *,int,const char *,const char *,
		int) ;
extern int mknpath3w(char *,int,const char *,const char *,const char *,
		int) ;
extern int mknpath4w(char *,int,const char *,const char *,const char *,
		const char *,int) ;
extern int mknpath5w(char *,int,const char *,const char *,const char *,
		const char *,const char *,int) ;
extern int mknpath6w(char *,int,const char *,const char *,const char *,
		const char *,const char *,const char *,int) ;


#ifdef	__cplusplus
}
#endif

#endif /* MKNPATH_INCLUDE	*/


