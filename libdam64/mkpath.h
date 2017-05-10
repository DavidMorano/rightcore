/* mkpath */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MKPATH_INCLUDE
#define	MKPATH_INCLUDE	1


#ifdef	__cplusplus
extern "C" {
#endif

extern int mkpath1(char *,const char *) ;
extern int mkpath2(char *,const char *,const char *) ;
extern int mkpath3(char *,const char *,const char *,const char *) ;
extern int mkpath4(char *,const char *,const char *,const char *,
		const char *) ;
extern int mkpath5(char *,const char *,const char *,const char *,
		const char *,const char *) ;
extern int mkpath6(char *,const char *,const char *,const char *,
		const char *,const char *,const char *) ;

extern int mkpath1w(char *,const char *,int) ;
extern int mkpath2w(char *,const char *,const char *,
		int) ;
extern int mkpath3w(char *,const char *,const char *,const char *,
		int) ;
extern int mkpath4w(char *,const char *,const char *,const char *,
		const char *,int) ;
extern int mkpath5w(char *,const char *,const char *,const char *,
		const char *,const char *,int) ;
extern int mkpath6w(char *,const char *,const char *,const char *,
		const char *,const char *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MKPATH_INCLUDE	*/


