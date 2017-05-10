/* bfliner */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	BFLINER_INCLUDE
#define	BFLINER_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<bfile.h>


#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	BFLINER		struct bfliner

struct bfliner {
	offset_t	poff ;		/* file-offset previous */
	offset_t	foff ;		/* file-offset current */
	bfile		*ifp ;
	char		*lbuf ;
	int		to ;		/* read time-out */
	int		ll ;
} ;


#if	(! defined(BFLINER_MASTER)) || (BFLINER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int bfliner_start(BFLINER *,bfile *,offset_t,int) ;
extern int bfliner_readpending(BFLINER *) ;
extern int bfliner_readline(BFLINER *,int,const char **) ;
extern int bfliner_readover(BFLINER *) ;
extern int bfliner_getpoff(BFLINER *,offset_t *) ;
extern int bfliner_finish(BFLINER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BFLINER_MASTER */

#endif /* BFLINER_INCLUDE */


