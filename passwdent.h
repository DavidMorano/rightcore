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

extern int passwdent_load(PASSWDENT *,char *,int,const PASSWDENT *) ;
extern int passwdent_parse(PASSWDENT *,char *,int,cchar *,int) ;
extern int passwdent_size(const PASSWDENT *) ;
extern int passwdent_format(const PASSWDENT *,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* PASSWDENT_INCLUDE */


