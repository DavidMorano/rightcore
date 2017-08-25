/* strmgr */

/* string manager */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STRMGR_INCLUDE
#define	STRMGR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	STRMGR		struct strmgr_head


struct strmgr_head {
	char		*dp ;
	int		dl ;
	int		dlen ;
} ;


typedef struct strmgr_head	strmgr ;


#if	(! defined(STRMGR_MASTER)) || (STRMGR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int strmgr_start(STRMGR *,char *,int) ;
extern int strmgr_avail(STRMGR *) ;
extern int strmgr_rem(STRMGR *) ;
extern int strmgr_str(STRMGR *,cchar *,int) ;
extern int strmgr_char(STRMGR *,int) ;
extern int strmgr_finish(STRMGR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRMGR_MASTER */

#endif /* STRMGR_INCLUDE */


