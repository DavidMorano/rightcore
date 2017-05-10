/* wordfill */

/* text fill */


/* revision history:

	= 1999-03-04, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	WORDFILL_INCLUDE
#define	WORDFILL_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>		/* base types */

#include	<fifostr.h>
#include	<localmisc.h>		/* extra types */


#define	WORDFILL_MAGIC	0x88442239
#define	WORDFILL	struct wordfill_head


struct wordfill_head {
	uint		magic ;
	fifostr		sq ;
	int		wc ;		/* word-count */
	int		cc ;		/* character count (w/ blanks) */
} ;


#if	(! defined(WORDFILL_MASTER)) || (WORDFILL_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	wordfill_start(WORDFILL *,const char *,int) ;
extern int	wordfill_addword(WORDFILL *,const char *,int) ;
extern int	wordfill_addline(WORDFILL *,const char *,int) ;
extern int	wordfill_addlines(WORDFILL *,const char *,int) ;
extern int	wordfill_mklinefull(WORDFILL *,char *,int) ;
extern int	wordfill_mklinepart(WORDFILL *,char *,int) ;
extern int	wordfill_finish(WORDFILL *) ;

#ifdef	__cplusplus
}
#endif

#endif /* WORDFILL_MASTER */

#endif /* WORDFILL_INCLUDE */


