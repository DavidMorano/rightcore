/* linehist */

/* line history */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LINEHIST_INCLUDE
#define	LINEHIST_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<langstate.h>
#include	<localmisc.h>


#define	LINEHIST_MAGIC		0x13f3c203
#define	LINEHIST		struct linehist_head
#define	LINEHIST_LINE		struct linehist_line


struct linehist_head {
	uint		magic ;
	LANGSTATE	ls ;
	void		*lvp ;		/* line history */
	char		ss[2] ;		/* search-characters */
} ;


#if	(! defined(LINEHIST_MASTER)) || (LINEHIST_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int linehist_start(LINEHIST *,cchar *) ;
extern int linehist_proc(LINEHIST *,int,cchar *,int) ;
extern int linehist_count(LINEHIST *) ;
extern int linehist_get(LINEHIST *,int,int *) ;
extern int linehist_finish(LINEHIST *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LINEHIST_MASTER */

#endif /* LINEHIST_INCLUDE */


