/* getpwentry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GETPWENTRY_INCLUDE
#define	GETPWENTRY_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<pwfile.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int getpwentry_name(PWENTRY *,char *,int,const char *) ;
extern int getpwentry_uid(PWENTRY *,char *,int,uid_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETPWENTRY_INCLUDE */


