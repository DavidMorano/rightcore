/* caldays */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	CALDAYS_INCLUDE
#define	CALDAYS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vechand.h>
#include	<vecstr.h>
#include	<localmisc.h>


#define	CALDAYS_MAGIC	0x99447245
#define	CALDAYS		struct caldays_head
#define	CALDAYS_OBJ	struct caldays_obj
#define	CALDAYS_QUERY	struct caldays_q
#define	CALDAYS_CITE	struct caldays_q
#define	CALDAYS_CUR	struct caldays_c
#define	CALDAYS_FL	struct caldays_flags


/* this is the shared-object description */
struct caldays_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct caldays_q {
	ushort		y ;
	uchar		m, d ;
} ;

struct caldays_c {
	uint		magic ;
	void		*results ;
	uint		nresults ;
	int		i ;
} ;

struct caldays_flags {
	uint		vind:1 ;		/* index is loaded */
} ;

struct caldays_head {
	uint		magic ;
	const char	*pr ;
	const char	*tmpdname ;
	char		**dirnames ;
	char		*dirstrtab ;
	CALDAYS_FL	f ;
	vechand		cals ;			/* calendars */
	vecstr		tmpfiles ;
	int		nentries ;
	int		ncursors ;
} ;


#if	(! defined(CALDAYS_MASTER)) || (CALDAYS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int caldays_open(CALDAYS *,const char *,
			const char **,const char **) ;
extern int caldays_count(CALDAYS *) ;
extern int caldays_curbegin(CALDAYS *,CALDAYS_CUR *) ;
extern int caldays_lookcite(CALDAYS *,CALDAYS_CUR *,CALDAYS_QUERY *) ;
extern int caldays_read(CALDAYS *,CALDAYS_CUR *,
			CALDAYS_CITE *,char *,int) ;
extern int caldays_curend(CALDAYS *,CALDAYS_CUR *) ;
extern int caldays_check(CALDAYS *,time_t) ;
extern int caldays_audit(CALDAYS *) ;
extern int caldays_close(CALDAYS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CALDAYS_MASTER */

#endif /* CALDAYS_INCLUDE */


