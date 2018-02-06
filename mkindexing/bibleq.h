/* bibleq */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BIBLEQ_INCLUDE
#define	BIBLEQ_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<modload.h>
#include	<localmisc.h>

#include	"bibleqs.h"


#define	BIBLEQ_MAGIC	0x99447243
#define	BIBLEQ		struct bibleq_head
#define	BIBLEQ_CUR	struct bibleq_c
#define	BIBLEQ_CALLS	struct bibleq_calls
#define	BIBLEQ_QUERY	BIBLEQS_CITE
#define	BIBLEQ_Q	BIBLEQS_CITE
#define	BIBLEQ_CITE	BIBLEQS_CITE

/* query options */

#define	BIBLEQ_OPREFIX	BIBLEQS_OPREFIX		/* prefix match */


struct bibleq_c {
	uint	magic ;
	void	*scp ;		/* SO-cursor pointer */
} ;

struct bibleq_calls {
	int	(*open)(void *,const char *,const char *) ;
	int	(*count)(void *) ;
	int	(*curbegin)(void *,void *) ;
	int	(*lookup)(void *,void *,int,const char **) ;
	int	(*enumerate)(void *,void *,BIBLEQS_CITE *,char *,int) ;
	int	(*curend)(void *,void *) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct bibleq_head {
	uint		magic ;
	MODLOAD		loader ;
	BIBLEQ_CALLS	call ;
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
	int		cursize ;	/* cursor size */
} ;


#if	(! defined(BIBLEQ_MASTER)) || (BIBLEQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int bibleq_open(BIBLEQ *,const char *,const char *) ;
extern int bibleq_count(BIBLEQ *) ;
extern int bibleq_curbegin(BIBLEQ *,BIBLEQ_CUR *) ;
extern int bibleq_lookup(BIBLEQ *,BIBLEQ_CUR *,int,const char **) ;
extern int bibleq_read(BIBLEQ *,BIBLEQ_CUR *,BIBLEQ_CITE *,char *,int) ;
extern int bibleq_curend(BIBLEQ *,BIBLEQ_CUR *) ;
extern int bibleq_audit(BIBLEQ *) ;
extern int bibleq_close(BIBLEQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLEQ_MASTER */

#endif /* BIBLEQ_INCLUDE */


