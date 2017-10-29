/* userattrent INCLUDE */

/* subroutines for simple USERATTR object (from UNIX® library-3c) management */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	USERATTRENT_INCLUDE
#define	USERATTRENT_INCLUDE	1


#include	<envstandards.h>
#include	<user_attr.h>


#define	USERATTRENT	userattr_t


#ifdef	__cplusplus
extern "C" {
#endif

extern int userattrent_load(userattr_t *,char *,int,const userattr_t *) ;
extern int userattrent_parse(userattr_t *,char *,int,cchar *,int) ;
extern int userattrent_size(const userattr_t *) ;
extern int userattrent_format(const userattr_t *,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* USERATTRENT_INCLUDE */


