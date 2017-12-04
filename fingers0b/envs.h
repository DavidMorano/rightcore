/* envs */

/* environment list container */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	ENVS_INCLUDE
#define	ENVS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<vecstr.h>
#include	<localmisc.h>


#define	ENVS		struct envs_head
#define	ENVS_CUR	struct envs_c
#define	ENVS_MAGIC	0x44376247


struct envs_c {
	HDB_CUR		cur ;
	int		i ;
} ;

struct envs_head {
	uint		magic ;
	HDB		vars ;
} ;


#if	(! defined(ENVS_MASTER)) || (ENVS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int envs_start(ENVS *,int) ;
extern int envs_store(ENVS *,cchar *,int,cchar *,int) ;
extern int envs_present(ENVS *,cchar *,int) ;
extern int envs_substr(ENVS *,cchar *,int,cchar *,int) ;
extern int envs_curbegin(ENVS *,ENVS_CUR *) ;
extern int envs_curend(ENVS *,ENVS_CUR *) ;
extern int envs_enumkey(ENVS *,ENVS_CUR *,cchar **) ;
extern int envs_enum(ENVS *,ENVS_CUR *,cchar **,cchar **) ;
extern int envs_fetch(ENVS *,cchar *,int,ENVS_CUR *,cchar **) ;
extern int envs_delname(ENVS *,cchar *,int) ;
extern int envs_count(ENVS *) ;
extern int envs_finish(ENVS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(ENVS_MASTER)) || (ENVS_MASTER == 0) */

#endif /* ENVS_INCLUDE */


