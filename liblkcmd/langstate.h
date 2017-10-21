/* langstate */

/* language (parse) state */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LANGSTATE_INCLUDE
#define	LANGSTATE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vecobj.h>
#include	<localmisc.h>


#define	LANGSTATE_MAGIC		0x13f3c204
#define	LANGSTATE		struct langstate_head
#define	LANGSTATE_FL		struct langstate_flags
#define	LANGSTATE_STAT		struct langstate_stat


enum langstatetypes {
	langstatetype_clear,
	langstatetype_comment,
	langstatetype_quote,
	langstatetype_literal,
	langstatetype_overlast
} ;

struct langstate_stat {
	int		line ;
	int		type ;
} ;

struct langstate_flags {
	uint		comment:1 ;
	uint		quote:1 ;
	uint		literal:1 ;
	uint		skip:1 ;
	uint		clear:1 ;
} ;

struct langstate_head {
	uint		magic ;
	LANGSTATE_FL	f ;
	int		line ;
	int		pch ;		/* previous character */
} ;


#if	(! defined(LANGSTATE_MASTER)) || (LANGSTATE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int langstate_start(LANGSTATE *) ;
extern int langstate_proc(LANGSTATE *,int,int) ;
extern int langstate_stat(LANGSTATE *,LANGSTATE_STAT *) ;
extern int langstate_finish(LANGSTATE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LANGSTATE_MASTER */

#endif /* LANGSTATE_INCLUDE */


