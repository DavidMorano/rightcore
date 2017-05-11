/* termout */

/* terminal-output management */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TERMOUT_INCLUDE
#define	TERMOUT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vecobj.h>
#include	<localmisc.h>


#define	TERMOUT_MAGIC	0x13f3c201
#define	TERMOUT		struct termout_head
#define	TERMOUT_FL	struct termout_flags


struct termout_flags {
	uint		isterm:1 ;
} ;

struct termout_head {
	uint		magic ;
	TERMOUT_FL	f ;
	void		*cvp ;		/* character-vector-pointer */
	void		*lvp ;		/* line-vector-pointer */
	uint		termattr ;	/* mask of terminal attributes */
	int		ncols ;		/* terminal columns */
	int		ncol ;
} ;


#if	(! defined(TERMOUT_MASTER)) || (TERMOUT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int termout_start(TERMOUT *,const char *,int,int) ;
extern int termout_load(TERMOUT *,const char *,int) ;
extern int termout_getline(TERMOUT *,int,const char **) ;
extern int termout_finish(TERMOUT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TERMOUT_MASTER */

#endif /* TERMOUT_INCLUDE */


