/* osetint */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	OSETINT_INCLUDE
#define	OSETINT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* for 'uint' */


#define	OSETINT		struct osetint_head
#define	OSETINT_CUR	struct osetint_cur


struct osetint_cur {
	void		*interp ;
} ;

struct osetint_head {
	void		*setp ;
} ;


typedef struct osetint_head	osetint ;
typedef struct osetint_cur	osetint_cur ;

#if	(! defined(OSETINT_MASTER)) || (OSETINT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int osetint_start(osetint *) ;
extern int osetint_finish(osetint *) ;
extern int osetint_addval(osetint *,int) ;
extern int osetint_delval(osetint *,int) ;
extern int osetint_count(osetint *) ;
extern int osetint_mkvec(osetint *,int *) ;
extern int osetint_extent(osetint *) ;
extern int osetint_curbegin(osetint *,osetint_cur *) ;
extern int osetint_enum(osetint *,osetint_cur *,int *) ;
extern int osetint_curend(osetint *,osetint_cur *) ;
extern int osetint_find(osetint *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* OSETINT_MASTER */

#endif /* OSETINT_INCLUDE */


