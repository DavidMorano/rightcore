/* termenq */


/* revision history:

	= 2000-07-19, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	TERMENQ_INCLUDE
#define	TERMENQ_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>	/* for special types */
#include	"terment.h"


/* object defines */

#define	TERMENQ		struct termenq_head
#define	TERMENQ_CUR	struct termenq_c
#define	TERMENQ_FL	struct termenq_flags
#define	TERMENQ_MAGIC	1092387456


struct termenq_c {
	int		i ;
} ;

struct termenq_flags {
	uint		writable:1 ;
} ;

struct termenq_head {
	uint		magic ;
	cchar		*fname ;	/* stored file name */
	caddr_t		mapdata ;	/* file mapping buffer */
	TERMENQ_FL	f ;
	time_t		ti_open ;	/* open time (for FD caching) */
	time_t		ti_mod ;	/* last modification time */
	time_t		ti_check ;	/* last check time */
	size_t		mapsize ;
	size_t		fsize ;		/* file total size */
	uint		mapoff ;	/* file mapping starting offset */
	int		pagesize ;
	int		oflags ;	/* open flags */
	int		operms ;	/* open permissions */
	int		fd ;		/* file descriptor */
	int		ncursors ;
	int		mapei ;		/* index of top mapped entry */
	int		mapen ;		/* number of mapped entries */
	int		en ;		/* convenience store */
} ;


#if	(! defined(TERMENQ_MASTER)) || (TERMENQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int termenq_open(TERMENQ *,cchar *,int) ;
extern int termenq_read(TERMENQ *,int,TERMENT *) ;
extern int termenq_write(TERMENQ *,int,TERMENT *) ;
extern int termenq_check(TERMENQ *,time_t) ;
extern int termenq_curbegin(TERMENQ *,TERMENQ_CUR *) ;
extern int termenq_curend(TERMENQ *,TERMENQ_CUR *) ;
extern int termenq_enum(TERMENQ *,TERMENQ_CUR *,TERMENT *) ;
extern int termenq_fetchline(TERMENQ *,TERMENQ_CUR *,TERMENT *,cchar *) ;
extern int termenq_fetchsid(TERMENQ *,TERMENT *,pid_t) ;
extern int termenq_nactive(TERMENQ *) ;
extern int termenq_close(TERMENQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TERMENQ_MASTER */

#endif /* TERMENQ_INCLUDE */


