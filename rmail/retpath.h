/* retpath */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	RETPATH_INCLUDE
#define	RETPATH_INCLUDE	1


#include	<envstandards.h>

#include	<sys/param.h>

#include	<vecstr.h>


#define	RETPATH		VECSTR


#if	(! defined(RETPATH_MASTER)) || (RETPATH_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int retpath_start(RETPATH *) ;
extern int retpath_parse(RETPATH *,const char *,int) ;
extern int retpath_add(RETPATH *,const char *,int) ;
extern int retpath_count(RETPATH *) ;
extern int retpath_search(RETPATH *,const char *,const char **) ;
extern int retpath_get(RETPATH *,int,const char **) ;
extern int retpath_mk(RETPATH *,char *,int) ;
extern int retpath_finish(RETPATH *) ;

#ifdef	__cplusplus
}
#endif

#endif /* RETPATH_MASTER */

#endif /* RETPATH_INCLUDE */


