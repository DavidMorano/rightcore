/* whitelist */

/* whitelist mail management */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	WHITELIST_INCLUDE
#define	WHITELIST_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>


#define	WHITELIST_MAGIC		0x65437296
#define	WHITELIST_DEFENTS	10
#define	WHITELIST		struct whitelist_head
#define	WHITELIST_CUR		struct whitelist_c


struct whitelist_head {
	uint		magic ;
	vecstr		list ;
} ;

struct whitelist_c {
	int		i ;
} ;


#if	(! defined(WHITELIST_MASTER)) || (WHITELIST_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int whitelist_open(WHITELIST *,const char *) ;
extern int whitelist_fileadd(WHITELIST *,const char *) ;
extern int whitelist_count(WHITELIST *) ;
extern int whitelist_get(WHITELIST *,int,const char **) ;
extern int whitelist_read(WHITELIST *,int,char *,int) ;
extern int whitelist_prematch(WHITELIST *,const char *) ;
extern int whitelist_audit(WHITELIST *) ;
extern int whitelist_close(WHITELIST *) ;

#ifdef	__cplusplus
}
#endif

#endif /* WHITELIST_MASTER */

#endif /* WHITELIST_INCLUDE */


