/* passwdent INCLUDE */

/* PASSWD structure management */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PASSWDENT_INCLUDE
#define	PASSWDENT_INCLUDE	1


#include	<envstandards.h>
#include	<pwd.h>


#define	PASSWDENT	struct passwd


#ifdef	__cplusplus
extern "C" {
#endif

extern int passwdent_load(struct passwd *,char *,int,const struct passwd *) ;
extern int passwdent_parse(struct passwd *,char *,int,const char *,int) ;
extern int passwdent_size(const struct passwd *) ;
extern int passwdent_format(const struct passwd *,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* PASSWDENT_INCLUDE */


