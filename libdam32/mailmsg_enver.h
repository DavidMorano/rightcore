/* mailmsg_enver */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGENVER_INCLUDE
#define	MAILMSGENVER_INCLUDE	1


#include	<envstandards.h>

#include	<sys/param.h>

#include	<mailmsg.h>


#define	MAILMSG_ENVER		struct mailmsg_enver


struct mailmsg_envstr {
	const char	*ep ;
	int		el ;
} ;

struct mailmsg_enver {
	struct mailmsg_envstr	a, d, r ;
} ;


#if	(! defined(MAILMSGENVER_MASTER)) || (MAILMSGENVER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsg_enver(MAILMSG *,int,MAILMSG_ENVER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGENVER_MASTER */

#endif /* MAILMSGENVER_INCLUDE */


