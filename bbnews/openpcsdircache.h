/* openpcsdircache */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	OPENPCSDIRCACHE_INCLUDE
#define	OPENPCSDIRCACHE_INCLUDE	1


#if	(! defined(OPENPCSDIRCACHE_MASTER)) || (OPENPCSDIRCACHE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int openpcsdircache(const char *,const char *,int,mode_t,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* OPENPCSDIRCACHE_MASTER */


#endif /* OPENPCSDIRCACHE_INCLUDE */



