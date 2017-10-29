/* osetstr */

/* Static-Set-String object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	OSETSTR_INCLUDE
#define	OSETSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecpstr.h>


#define	OSETSTR_MAGIC	0x09097641
#define	OSETSTR_CSIZE	10
#define	OSETSTR		struct osetstr_head
#define	OSETSTR_CUR	struct osetstr_cur


struct osetstr_head {
	uint		magic ;
	vecpstr		ents ;
} ;

struct osetstr_cur {
	int		i ;
} ;


typedef	OSETSTR		osetstr ;
typedef	OSETSTR_CUR	osetstr_cur ;


#if	(! defined(OSETSTR_MASTER)) || (OSETSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int osetstr_start(OSETSTR *,int) ;
extern int osetstr_look(OSETSTR *,const char *,int) ;
extern int osetstr_add(OSETSTR *,const char *,int) ;
extern int osetstr_curbegin(OSETSTR *,OSETSTR_CUR *) ;
extern int osetstr_curend(OSETSTR *,OSETSTR_CUR *) ;
extern int osetstr_curdel(OSETSTR *,OSETSTR_CUR *) ;
extern int osetstr_findn(OSETSTR *,const char *,int) ;
extern int osetstr_enum(OSETSTR *,OSETSTR_CUR *,const char **) ;
extern int osetstr_next(OSETSTR *,OSETSTR_CUR *) ;
extern int osetstr_count(OSETSTR *) ;
extern int osetstr_finish(OSETSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* OSETSTR_MASTER */

#endif /* OSETSTR_INCLUDE */


