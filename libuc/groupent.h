/* groupent INCLUDE */

/* subroutines for simple GROUP object (from UNIX® library-3c) management */


/* revision history:

	= 1998-02-12, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GROUPENT_INCLUDE
#define	GROUPENT_INCLUDE	1


#include	<envstandards.h>
#include	<grp.h>


#define	GROUPENT	struct group


#ifdef	__cplusplus
extern "C" {
#endif

extern int groupent_load(struct group *,char *,int,const struct group *) ;
extern int groupent_parse(struct group *,char *,int,const char *,int) ;
extern int groupent_size(const struct group *) ;
extern int groupent_format(const struct group *,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* GROUPENT_INCLUDE */


