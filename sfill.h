/* sfill */

/* text fill */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SFILL_INCLUDE
#define	SFILL_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<bfile.h>
#include	<fifostr.h>


#define	SFILL		struct sfill_head


struct sfill_head {
	bfile		*ofp ;
	fifostr		sq ;
	int		clen ;
	int		indent ;
} ;


#if	(! defined(SFILL_MASTER)) || (SFILL_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sfill_start(SFILL *,int,bfile *) ;
extern int	sfill_remaining(SFILL *) ;
extern int	sfill_proc(SFILL *,int,const char *,int) ;
extern int	sfill_wline(SFILL *,int) ;
extern int	sfill_finish(SFILL *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SFILL_MASTER */

#endif /* SFILL_INCLUDE */


