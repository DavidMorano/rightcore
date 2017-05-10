/* holidayer */


/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

#ifndef	HOLIDAYER_INCLUDE
#define	HOLIDAYER_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<vechand.h>
#include	<localmisc.h>

#include	"holidays.h"


#define	HOLIDAYER_MAGIC	0x63328184
#define	HOLIDAYER	struct holidayer_head
#define	HOLIDAYER_OBJ	struct holidayer_obj
#define	HOLIDAYER_CUR	struct holidayer_c
#define	HOLIDAYER_FL	struct holidayer_flags
#define	HOLIDAYER_CITE	HOLIDAYS_CITE


/* this is the shared-object description */
struct holidayer_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct holidayer_c {
	uint		magic ;
	HOLIDAYS_CUR	hcur ;
	HOLIDAYS	*hop ;
	uint		year ;
} ;

struct holidayer_flags {
	uint		hols:1 ;
} ;

struct holidayer_head {
	uint		magic ;
	cchar		*pr ;
	HOLIDAYER_FL	f ;
	IDS		id ;
	vechand		hols ;
	uint		year ;
	int		ncursors ;
} ;


#if	(! defined(HOLIDAYER_MASTER)) || (HOLIDAYER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int holidayer_open(HOLIDAYER *,cchar *) ;
extern int holidayer_curbegin(HOLIDAYER *,HOLIDAYER_CUR *) ;
extern int holidayer_curend(HOLIDAYER *,HOLIDAYER_CUR *) ;
extern int holidayer_fetchcite(HOLIDAYER *,HOLIDAYER_CITE *,HOLIDAYER_CUR *,
		char *,int) ;
extern int holidayer_fetchname(HOLIDAYER *,uint,cchar *,int,HOLIDAYER_CUR *,
		HOLIDAYER_CITE *,char *,int) ;
extern int holidayer_enum(HOLIDAYER *,HOLIDAYER_CUR *,
		HOLIDAYER_CITE *,char *,int,uint) ;
extern int holidayer_check(HOLIDAYER *,time_t) ;
extern int holidayer_audit(HOLIDAYER *) ;
extern int holidayer_close(HOLIDAYER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* HOLIDAYER_MASTER */

#endif /* HOLIDAYER_INCLUDE */


