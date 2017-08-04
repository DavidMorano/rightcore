/* setint */
/* lang=C++11 */


/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

#ifndef	SETINT_INCLUDE
#define	SETINT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* for 'uint' */


#define	SETINT		struct setint_head
#define	SETINT_CUR	struct setint_cur


struct setint_cur {
	void		*interp ;
} ;

struct setint_head {
	void		*setp ;
} ;


typedef struct setint_head	setint ;
typedef struct setint_cur	setint_cur ;

#if	(! defined(SETINT_MASTER)) || (SETINT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int setint_start(setint *) ;
extern int setint_finish(setint *) ;
extern int setint_addval(setint *,int) ;
extern int setint_delval(setint *,int) ;
extern int setint_count(setint *) ;
extern int setint_mkvec(setint *,int *) ;
extern int setint_extent(setint *) ;
extern int setint_curbegin(setint *,setint_cur *) ;
extern int setint_enum(setint *,setint_cur *,int *) ;
extern int setint_curend(setint *,setint_cur *) ;
extern int setint_find(setint *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SETINT_MASTER */

#endif /* SETINT_INCLUDE */


