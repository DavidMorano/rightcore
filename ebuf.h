/* ebuf */

/* last modified %G% version %I% */


/* revision history:

	= 2003-10-22, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

#ifndef	EBUF_INCLUDE
#define	EBUF_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


/* object defines */

#define	EBUF_MAGIC	1092847456
#define	EBUF_NENTS	4
#define	EBUF		struct ebuf_head
#define	EBUF_FL		struct ebuf_flags
#define	EBUF_WAY	struct ebuf_way


/* file buffer state */
struct ebuf_way {
	offset_t	woff ;		/* "way" offset to file entries */
	char		*wbuf ;		/* buffer */
	uint		utime ;		/* usage time */
	int		wlen ;
	int		nvalid ;	/* number of valid entries */
} ;

struct ebuf_flags {
	uint		init:1 ;	/* init'ed */
} ;

struct ebuf_head {
	uint		magic ;
	EBUF_FL		f ;
	EBUF_WAY	*ways ;
	uint		utimer ;	/* usage timer (fake time) */
	uint		soff ;		/* starting offset */
	int		esize ;		/* entry size */
	int		nways ;		/* maximum number of ways */
	int		iways ;		/* active number of ways */
	int		npw ;		/* number entries per way */
	int		nentries ;	/* number of total entries */
	int		fd ;
} ;


#if	(! defined(EBUF_MASTER)) || (EBUF_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ebuf_start(EBUF *,int,uint,int,int,int) ;
extern int ebuf_finish(EBUF *) ;
extern int ebuf_read(EBUF *,int,char **) ;
extern int ebuf_write(EBUF *,int,const void *) ;
extern int ebuf_count(EBUF *) ;
extern int ebuf_sync(EBUF *) ;
extern int ebuf_invalidate(EBUF *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* EBUF_MASTER */

#endif /* EBUF_INCLUDE */


