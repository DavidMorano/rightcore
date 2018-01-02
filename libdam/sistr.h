/* sistr */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SISTR_INCLUDE
#define	SISTR_INCLUDE	1


#ifdef	__cplusplus
extern "C" {
#endif

extern int sichr(const char *,int,int) ;
extern int sialpha(const char *,int) ;
extern int sidigit(const char *,int) ;
extern int sialnum(const char *,int) ;
extern int sibasename(const char *,int) ;
extern int sibreak(const char *,int,const char *) ;
extern int sibrk(const char *,int,const char *) ;
extern int sicasesub(const char *,int,const char *) ;
extern int sihyphen(const char *,int) ;
extern int siskipwhite(const char *,int) ;
extern int sispan(const char *,int,const char *) ;
extern int sisub(const char *,int,const char *) ;
extern int sirec(const char *,int,int) ;

extern int substring(const char *,int,const char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SISTR_INCLUDE	*/


