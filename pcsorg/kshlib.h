/* kshlib */


/* revision history:

	= 2001-11-01, David A­D­ Morano

	Written to have a place for the various KSH initialization
	subroutines.


*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

#ifndef	KSHLIB_INCLUDE
#define	KSHLIB_INCLUDE	1


#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


#define	KSHLIB_NENTS	30


#ifdef	__cplusplus
extern "C" {
#endif

extern int	lib_initenviron(void) ;
extern int	lib_initmemalloc(int) ;
extern int	lib_mainbegin(const char **) ;
extern int	lib_mainend(void) ;
extern int	lib_kshbegin(void *) ;
extern int	lib_kshend(void) ;
extern int	lib_runmode(void) ;
extern int	lib_sigintr(void) ;
extern int	lib_sigterm(void) ;
extern int	lib_sigreset(int) ;
extern int	lib_callfunc(void *,int,cchar **,cchar **,void *) ;
extern int	lib_callprog(cchar *,int,cchar **,cchar **,void *) ;
extern int	lib_runbegin(const char **) ;
extern int	lib_runend(void) ;

#ifdef	__cplusplus
}
#endif

#endif /* KSHLIB_INCLUDE */


