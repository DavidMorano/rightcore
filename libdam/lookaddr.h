/* lookaddr */


/* revision history:

	= 2002-05-17, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

#ifndef	LOOKADDR_INCLUDE
#define	LOOKADDR_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vecstr.h>
#include	<whitelist.h>
#include	<localmisc.h>


#define	LOOKADDR_MAGIC		0x97658232
#define	LOOKADDR		struct lookaddr_head
#define	LOOKADDR_SL		struct lookaddr_sysflags
#define	LOOKADDR_USER		struct lookaddr_user
#define	LOOKADDR_UF		struct lookaddr_userflags


struct lookaddr_sysflags {
	uint		swl:1 ;
	uint		sbl:1 ;
} ;

struct lookaddr_userflags {
	uint		dname:1 ;
	uint		uwl:1 ;
	uint		ubl:1 ;
} ;

struct lookaddr_user {
	uint		magic ;
	cchar		*dname ;
	LOOKADDR_UF	init, open ;
	WHITELIST	uwl ;
	WHITELIST	ubl ;
} ;

struct lookaddr_head {
	uint		magic ;
	LOOKADDR_SL	init, open ;
	VECSTR		sv ;
	WHITELIST	swl ;
	WHITELIST	sbl ;
	int		users ;
} ;


#if	(! defined(LOOKADDR_MASTER)) || (LOOKADDR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int lookaddr_start(LOOKADDR *,cchar *,cchar *) ;
extern int lookaddr_userbegin(LOOKADDR *,LOOKADDR_USER *,cchar *) ;
extern int lookaddr_usercheck(LOOKADDR *,LOOKADDR_USER *,cchar *,int) ;
extern int lookaddr_userend(LOOKADDR *,LOOKADDR_USER *) ;
extern int lookaddr_finish(LOOKADDR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LOOKADDR_MASTER */

#endif /* LOOKADDR_INCLUDE */


