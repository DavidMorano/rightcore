/* osetstr */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	OSETSTR_INCLUDE
#define	OSETSTR_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* for 'uint' */


#define	OSETSTR		struct osetstr_head
#define	OSETSTR_CUR	struct osetstr_c


struct osetstr_c {
	void		*interp ;
} ;

struct osetstr_head {
	void		*setp ;
} ;


typedef struct osetstr_head	osetstr ;
typedef struct osetstr_c	osetstr_cur ;


#if	(! defined(OSETSTR_MASTER)) || (OSETSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int osetstr_start(osetstr *,int) ;
extern int osetstr_already(osetstr *,cchar *,int) ;
extern int osetstr_add(osetstr *,cchar *,int) ;
extern int osetstr_del(osetstr *,cchar *,int) ;
extern int osetstr_count(osetstr *) ;
extern int osetstr_curbegin(osetstr *,osetstr_cur *) ;
extern int osetstr_enum(osetstr *,osetstr_cur *,cchar **) ;
extern int osetstr_curend(osetstr *,osetstr_cur *) ;
extern int osetstr_finish(osetstr *) ;

#ifdef	__cplusplus
}
#endif

#endif /* OSETSTR_MASTER */

#endif /* OSETSTR_INCLUDE */


