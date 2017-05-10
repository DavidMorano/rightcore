/* shadowent INCLUDE */

/* SHADOW structure management */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SHADOWENT_INCLUDE
#define	SHADOWENT_INCLUDE	1


#include	<envstandards.h>
#include	<shadow.h>


#define	SHADOWNT	struct spwd


#ifdef	__cplusplus
extern "C" {
#endif

extern int shadowent_load(struct spwd *,char *,int,const struct spwd *) ;
extern int shadowent_parse(struct spwd *,char *,int,const char *,int) ;
extern int shadowent_size(const struct spwd *) ;
extern int shadowent_format(const struct spwd *,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SHADOWENT_INCLUDE */


