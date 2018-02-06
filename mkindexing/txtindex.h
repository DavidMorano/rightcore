/* txtindex */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	TXTINDEX_INCLUDE
#define	TXTINDEX_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<modload.h>
#include	<localmisc.h>
#include	"txtindexes.h"


#define	TXTINDEX_MAGIC		0x99447246
#define	TXTINDEX		struct txtindex_head
#define	TXTINDEX_CUR		struct txtindex_c
#define	TXTINDEX_CALLS		struct txtindex_calls
#define	TXTINDEX_TAG		TXTINDEXES_TAG
#define	TXTINDEX_INFO		TXTINDEXES_INFO


struct txtindex_c {
	uint	magic ;
	void	*scp ;		/* SO-cursor pointer */
} ;

struct txtindex_calls {
	int	(*open)(void *,const char *) ;
	int	(*count)(void *) ;
	int	(*neigen)(void *) ;
	int	(*info)(void *,TXTINDEXES_INFO *) ;
	int	(*iseigen)(void *,const char *,int) ;
	int	(*curbegin)(void *,void *) ;
	int	(*lookup)(void *,void *,const char **) ;
	int	(*read)(void *,void *,TXTINDEXES_TAG *) ;
	int	(*curend)(void *,void *) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct txtindex_head {
	uint		magic ;
	MODLOAD		loader ;
	TXTINDEX_CALLS	call ;
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
	int		cursize ;	/* cursor size */
} ;


#if	(! defined(TXTINDEX_MASTER)) || (TXTINDEX_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	txtindex_open(TXTINDEX *,const char *,const char *) ;
extern int	txtindex_count(TXTINDEX *) ;
extern int	txtindex_neigen(TXTINDEX *) ;
extern int	txtindex_info(TXTINDEX *,TXTINDEX_INFO *) ;
extern int	txtindex_iseigen(TXTINDEX *,const char *,int) ;
extern int	txtindex_curbegin(TXTINDEX *,TXTINDEX_CUR *) ;
extern int	txtindex_lookup(TXTINDEX *,TXTINDEX_CUR *,cchar **) ;
extern int	txtindex_read(TXTINDEX *,TXTINDEX_CUR *,TXTINDEX_TAG *) ;
extern int	txtindex_curend(TXTINDEX *,TXTINDEX_CUR *) ;
extern int	txtindex_audit(TXTINDEX *) ;
extern int	txtindex_close(TXTINDEX *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TXTINDEX_MASTER */

#endif /* TXTINDEX_INCLUDE */


