/* comsatmsg */

/* create and parse COMSAT messages */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	COMSATMSG_INCLUDE
#define	COMSATMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#define	COMSATMSG_MO	struct comsatmsg_mo


struct comsatmsg_mo {
	ulong		offset ;
	char		username[USERNAMELEN + 1] ;
	char		fname[MAXNAMELEN + 1] ;
} ;


/* message types */
enum comsatmsgtypes {
	comsatmsgtype_mo,
	comsatmsgtype_overlast
} ;


#if	(! defined(COMSATMSG_MASTER)) || (COMSATMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int comsatmsg_mo(COMSATMSG_MO *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* COMSATMSG_MASTER */

#endif /* COMSATMSG_INCLUDE */


