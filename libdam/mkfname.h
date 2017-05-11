/* mkfname */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MKFNAME_INCLUDE
#define	MKFNAME_INCLUDE		1


#ifdef	__cplusplus
extern "C" {
#endif

extern int mkfname(char *,int,const char *,...) ;
extern int mkfname1(char *,const char *,const char *) ;
extern int mkfname2(char *,const char *,const char *,const char *) ;
extern int mkfname3(char *,const char *,const char *,const char *,
		const char *) ;
extern int mkfname4(char *,const char *,const char *,const char *,
		const char *,const char *) ;
extern int mkfname5(char *,const char *,const char *,const char *,
		const char *,const char *,const char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MKFNAME_INCLUDE	*/


