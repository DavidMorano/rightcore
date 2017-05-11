/* malloclog */

/* malloc logging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MALLOCLOG_INCLUDE
#define	MALLOCLOG_INCLUDE	1


#if	(! defined(MALLOCLOG_MASTER)) || (MALLOCLOG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern void	malloclog_alloc(const void *,int,const char *) ;
extern void	malloclog_free(const void *,const char *) ;
extern void	malloclog_realloc(const void *,const void *,int,const char *) ;
extern void	malloclog_mark() ;
extern void	malloclog_dump() ;
extern void	malloclog_clear() ;
extern int	malloclog_printf(const char [],...) ;

#ifdef	__cplusplus
}
#endif

#endif /* MALLOCLOG_MASTER */

#endif /* MALLOCLOG_INCLUDE */


