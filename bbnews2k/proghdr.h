/* proghdr */


/* Copyright © 1995,1998,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	PROGHDR_INCLUDE
#define	PROGHDR_INCLUDE		1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>		/* extra types */


#if	(! defined(PROGHDR_MASTER)) || (PROGHDR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int proghdr_begin(PROGINFO *) ;
extern int proghdr_end(PROGINFO *) ;
extern int proghdr_trans(PROGINFO *,char *,int,cchar *,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROGHDR_MASTER */

#endif /* PROGHDR_INCLUDE */


