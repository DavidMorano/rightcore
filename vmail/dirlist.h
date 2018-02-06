/* dirlist */


/* revision history:

	= 2000-09-10, David A­D­ Morano
        I created this modeled after something similar that was used for some of
        my PCS programs.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	DIRLIST_INCLUDE
#define	DIRLIST_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	DIRLIST		struct dirlist_head
#define	DIRLIST_CUR	struct dirlist_c
#define	DIRLIST_MAGIC	0x09854123


struct dirlist_c {
	int		i ;
} ;

struct dirlist_head {
	uint		magic ;
	VECOBJ		db ;
	int		tlen ;
} ;


#if	(! defined(DIRLIST_MASTER)) || (DIRLIST_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dirlist_start(DIRLIST *) ;
extern int dirlist_semi(DIRLIST *) ;
extern int dirlist_adds(DIRLIST *,const char *,int) ;
extern int dirlist_add(DIRLIST *,const char *,int) ;
extern int dirlist_strsize(DIRLIST *) ;
extern int dirlist_curbegin(DIRLIST *,DIRLIST_CUR *) ;
extern int dirlist_curend(DIRLIST *,DIRLIST_CUR *) ;
extern int dirlist_enum(DIRLIST *,DIRLIST_CUR *,char *,int) ;
extern int dirlist_get(DIRLIST *,DIRLIST_CUR *,const char **) ;
extern int dirlist_join(DIRLIST *) ;
extern int dirlist_joinsize(DIRLIST *) ;
extern int dirlist_joinmk(DIRLIST *,char *,int) ;
extern int dirlist_finish(DIRLIST *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DIRLIST_MASTER */

#endif /* DIRLIST_INCLUDE */


