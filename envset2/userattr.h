/* userattr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	USERATTR_INCLUDE
#define	USERATTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<user_attr.h>

#include	<localmisc.h>


/* miscellaneous defines */

/* object defines */

#define	USERATTR		struct userattr_head
#define	USERATTR_FL		struct userattr_flags
#define	USERATTR_MAGIC		0x99999898


struct userattr_flags {
	uint		sysdb:1 ;		/* OS database */
	uint		udomain:1 ;		/* UDOMAIN database */
} ;

struct userattr_head {
	uint		magic ;
	const char	*username ;	/* passed argument */
	const char	*domain ;	/* cached */
	userattr_t	*uap ;		/* returned (allocated) */
	USERATTR_FL	init, have ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int userattr_open(USERATTR *,const char *) ;
extern int userattr_lookup(USERATTR *,char *,int,const char *) ;
extern int userattr_close(USERATTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* USERATTR_INCLUDE */


