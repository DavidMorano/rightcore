/* sfstr */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SFSTR_INCLUDE
#define	SFSTR_INCLUDE	1


#ifdef	__cplusplus
extern "C" {
#endif

extern int sfshrink(const char *,int,const char **) ;
extern int sfbasename(const char *,int,const char **) ;
extern int sfdirname(const char *,int,const char **) ;
extern int sfbreak(const char *,int,const char *,const char **) ;
extern int sfcasesub(const char *,int,const char *,const char **) ;
extern int sfdequote(const char *,int,const char **) ;
extern int sfkey(const char *,int,const char **) ;
extern int sfprogroot(const char *,int,const char **) ;
extern int sfskipwhite(const char *,int,const char **) ;
extern int sfsub(const char *,int,const char *,const char **) ;
extern int sfthing(const char *,int,const char *,const char **) ;
extern int sfwhitedot(const char *,int,const char **) ;
extern int sfword(const char *,int,const char **) ;
extern int sfnext(const char *,int,const char **) ;

extern int nextfield(const char *,int,const char **) ;

#ifdef	__cplusplus
}
#endif

#endif /* SFSTR_INCLUDE	*/


