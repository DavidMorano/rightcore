/* txtindexmk */


/* revision history:

	= 2008-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	TXTINDEXMK_INCLUDE
#define	TXTINDEXMK_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<modload.h>
#include	<localmisc.h>

#include	"txtindexmks.h"


#define	TXTINDEXMK_MAGIC	0x99447246
#define	TXTINDEXMK		struct txtindexmk_head
#define	TXTINDEXMK_ENTS		struct txtindexmk_ents
#define	TXTINDEXMK_PA		TXTINDEXMKS_PA
#define	TXTINDEXMK_TAG		TXTINDEXMKS_TAG
#define	TXTINDEXMK_KEY		TXTINDEXMKS_KEY


struct txtindexmk_ents {
	int	(*open)(void *,TXTINDEXMKS_PA *,const char *,int,int) ;
	int	(*addeigens)(void *,TXTINDEXMKS_KEY *,int) ;
	int	(*addtags)(void *,TXTINDEXMKS_TAG *,int) ;
	int	(*noop)(void *) ;
	int	(*abort)(void *) ;
	int	(*close)(void *) ;
} ;

struct txtindexmk_head {
	uint		magic ;
	MODLOAD		loader ;
	TXTINDEXMK_ENTS	call ;
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
	int		cursize ;	/* cursor size (not used here) */
	int		nfd ;
} ;


#if	(! defined(TXTINDEXMK_MASTER)) || (TXTINDEXMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int txtindexmk_open(TXTINDEXMK *,TXTINDEXMK_PA *,cchar *,int,mode_t) ;
extern int txtindexmk_addeigens(TXTINDEXMK *,TXTINDEXMK_KEY *,int) ;
extern int txtindexmk_addtags(TXTINDEXMK *,TXTINDEXMK_TAG *,int) ;
extern int txtindexmk_noop(TXTINDEXMK *) ;
extern int txtindexmk_abort(TXTINDEXMK *) ;
extern int txtindexmk_close(TXTINDEXMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TXTINDEXMK_MASTER */

#endif /* TXTINDEXMK_INCLUDE */


