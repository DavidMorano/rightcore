/* calyears */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	CALYEARS_INCLUDE
#define	CALYEARS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vechand.h>
#include	<vecobj.h>
#include	<holidayer.h>
#include	<localmisc.h>
#include	"calent.h"
#include	"calcite.h"


#define	CALYEARS_MAGIC	0x99447245
#define	CALYEARS	struct calyears_head
#define	CALYEARS_OBJ	struct calyears_obj
#define	CALYEARS_Q	CALCITE
#define	CALYEARS_QUERY	CALCITE
#define	CALYEARS_CITE	CALCITE
#define	CALYEARS_CUR	struct calyears_c
#define	CALYEARS_FL	struct calyears_flags


/* this is the shared-object description */
struct calyears_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct calyears_c {
	uint		magic ;
	void		*results ;
	uint		nresults ;
	int		i ;
} ;

struct calyears_flags {
	uint		doms:1 ;		/* day-of-month(s) */
	uint		hols:1 ;		/* holidays */
} ;

struct calyears_head {
	uint		magic ;
	cchar		*a ;			/* memory-allocation */
	cchar		*pr ;
	cchar		*tmpdname ;
	CALYEARS_FL	f ;
	CALYEARS_FL	init, open ;
	HOLIDAYER	hols ;
	vechand		doms ;
	vechand		cals ;			/* calendars */
	int		nentries ;
	int		ncursors ;
	int		year ;			/* current year */
	int		isdst ;			/* current is-dst */
	int		gmtoff ;		/* current offset from GMT */
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
extern int calyears_already(CALYEARS *,vecobj *,CALENT *) ;
extern int calyears_havestart(CALYEARS *,CALCITE *,int,cchar *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* CALYEARS_MASTER */

#endif /* CALYEARS_INCLUDE */


