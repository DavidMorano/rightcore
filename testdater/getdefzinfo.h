/* getdefzinfo */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GETDEFZINFO_INCLUDE
#define	GETDEFZINFO_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>


#define	GETDEFZINFO		struct getdefzinfo
#define	GETDEFZINFO_ZNAMESIZE	8


struct getdefzinfo {
	int	zoff  ;		/* minutes west of GMT */
	int	isdst ;		/* is-dst flag */
	char	zname[GETDEFZINFO_ZNAMESIZE + 1] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	getdefzinfo(GETDEFZINFO *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETDEFZINFO_INCLUDE */


