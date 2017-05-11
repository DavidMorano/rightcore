/* netfile */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	NETFILE_INCLUDE
#define	NETFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecitem.h>
#include	<localmisc.h>


/* object defines */

#define	NETFILE		VECITEM
#define	NETFILE_ENT	struct netfile_ent


struct netfile_ent {
	const char	*machine ;		/* machine name */
	const char	*login ;		/* login name */
	const char	*password ;
	const char	*account ;		/* account name */
} ;


#if	(! defined(NETFILE_MASTER)) || (NETFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int netfile_open(NETFILE *,const char *) ;
extern int netfile_get(NETFILE *,int,NETFILE_ENT **) ;
extern int netfile_close(NETFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* NETFILE_MASTER */

#endif /* NETFILE_INCLUDE */


