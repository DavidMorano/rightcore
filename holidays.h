/* holidays */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	HOLIDAYS_INCLUDE
#define	HOLIDAYS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>


#define	HOLIDAYS_MAGIC	0x63328183
#define	HOLIDAYS	struct holidays_head
#define	HOLIDAYS_OBJ	struct holidays_obj
#define	HOLIDAYS_CITE	struct holidays_q
#define	HOLIDAYS_CUR	struct holidays_c


/* this is the shared-object description */
struct holidays_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct holidays_q {
	ushort		y ;
	uchar		m, d ;
} ;

struct holidays_c {
	uint		chash ;
	int		i ;
} ;

struct holidays_head {
	uint		magic ;
	const char	*pr ;
	const char	*fname ;
	uint		(*rt)[3] ;	/* the records (sorted by m-d) */
	int		(*kit)[3] ;	/* key-index-table */
	char		*kst ;		/* key string-table */
	char		*vst ;		/* val string-table */
	time_t		ti_check ;
	time_t		ti_mtime ;
	int		year ;
	int		kslen ;
	int		vslen ;
	int		rtlen ;
	int		itlen ;
	int		ncursors ;
} ;


#if	(! defined(HOLIDAYS_MASTER)) || (HOLIDAYS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int holidays_open(HOLIDAYS *,const char *,int,const char *) ;
extern int holidays_count(HOLIDAYS *) ;
extern int holidays_curbegin(HOLIDAYS *,HOLIDAYS_CUR *) ;
extern int holidays_curend(HOLIDAYS *,HOLIDAYS_CUR *) ;
extern int holidays_fetchcite(HOLIDAYS *,HOLIDAYS_CITE *,HOLIDAYS_CUR *,
		char *,int) ;
extern int holidays_fetchname(HOLIDAYS *,const char *,int,HOLIDAYS_CUR *,
		HOLIDAYS_CITE *,char *,int) ;
extern int holidays_enum(HOLIDAYS *,HOLIDAYS_CUR *,
		HOLIDAYS_CITE *,char *,int) ;
extern int holidays_check(HOLIDAYS *,time_t) ;
extern int holidays_audit(HOLIDAYS *) ;
extern int holidays_close(HOLIDAYS *) ;

#ifdef	COMMENT
extern int holidays_lookday(HOLIDAYS *,HOLIDAYS_CUR *,HOLIDAYS_CITE *) ;
extern int holidays_lookname(HOLIDAYS *,HOLIDAYS_CUR *,const char *,int) ;
extern int holidays_read(HOLIDAYS *,HOLIDAYS_CUR *,
			HOLIDAYS_CITE *,char *,int) ;
#endif

#ifdef	__cplusplus
}
#endif

#endif /* HOLIDAYS_MASTER */

#endif /* HOLIDAYS_INCLUDE */


