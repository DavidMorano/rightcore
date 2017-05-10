/* calyears */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	CALYEARS_INCLUDE
#define	CALYEARS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vechand.h>
#include	<vecstr.h>
#include	<localmisc.h>


#define	CALYEARS_MAGIC	0x99447245
#define	CALYEARS	struct calyears_head
#define	CALYEARS_OBJ	struct calyears_obj
#define	CALYEARS_Q	struct calyears_q
#define	CALYEARS_QUERY	struct calyears_q
#define	CALYEARS_CITE	struct calyears_q
#define	CALYEARS_CUR	struct calyears_c
#define	CALYEARS_FL	struct calyears_flags


/* this is the shared-object description */
struct calyears_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct calyears_q {
	ushort		y ;
	uchar		m, d ;
} ;

struct calyears_c {
	uint		magic ;
	void		*results ;
	uint		nresults ;
	int		i ;
} ;

struct calyears_flags {
	uint		vind:1 ;		/* index is loaded */
} ;

struct calyears_head {
	uint		magic ;
	const char	*pr ;
	const char	*tmpdname ;
	char		**dirnames ;
	char		*dirstrtab ;
	CALYEARS_FL	f ;
	vechand		cals ;			/* calendars */
	vecstr		tmpfiles ;
	int		nentries ;
	int		ncursors ;
} ;


#if	(! defined(CALYEARS_MASTER)) || (CALYEARS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int calyears_open(CALYEARS *,cchar *,cchar **,cchar **) ;
extern int calyears_count(CALYEARS *) ;
extern int calyears_curbegin(CALYEARS *,CALYEARS_CUR *) ;
extern int calyears_lookcite(CALYEARS *,CALYEARS_CUR *,CALYEARS_Q *) ;
extern int calyears_read(CALYEARS *,CALYEARS_CUR *,CALYEARS_Q *,char *,int) ;
extern int calyears_curend(CALYEARS *,CALYEARS_CUR *) ;
extern int calyears_check(CALYEARS *,time_t) ;
extern int calyears_audit(CALYEARS *) ;
extern int calyears_close(CALYEARS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CALYEARS_MASTER */

#endif /* CALYEARS_INCLUDE */


