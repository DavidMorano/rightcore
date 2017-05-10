/* envlist */

/* environment container */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	ENVLIST_INCLUDE
#define	ENVLIST_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<localmisc.h>


/* object defines */

#define	ENVLIST			struct envlist_head
#define	ENVLIST_CHUNKSIZE	256


struct envlist_head {
	HDB		list ;
	void		*store ;
} ;


typedef ENVLIST		envlist ;


#if	(! defined(ENVLIST_MASTER)) || (ENVLIST_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int envlist_start(ENVLIST *,int) ;
extern int envlist_addkeyval(ENVLIST *,cchar *,cchar *,int) ;
extern int envlist_add(ENVLIST *,cchar *,int) ;
extern int envlist_present(ENVLIST *,cchar *,int,cchar **) ;
extern int envlist_count(ENVLIST *) ;
extern int envlist_finish(ENVLIST *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(ENVLIST_MASTER)) || (ENVLIST_MASTER == 0) */

#endif /* ENVLIST_INCLUDE */


