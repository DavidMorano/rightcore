/* vsetstr */

/* Vector-Implemented Ordered-Set-String object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VSETSTR_INCLUDE
#define	VSETSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecpstr.h>


#define	VSETSTR_MAGIC	0x09097641
#define	VSETSTR_CSIZE	10
#define	VSETSTR		struct vsetstr_head
#define	VSETSTR_CUR	struct vsetstr_cur


struct vsetstr_head {
	uint		magic ;
	vecpstr		ents ;
} ;

struct vsetstr_cur {
	int		i ;
} ;


typedef	VSETSTR		vsetstr ;
typedef	VSETSTR_CUR	vsetstr_cur ;


#if	(! defined(VSETSTR_MASTER)) || (VSETSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int vsetstr_start(VSETSTR *,int) ;
extern int vsetstr_already(VSETSTR *,cchar *,int) ;
extern int vsetstr_del(VSETSTR *,cchar *,int) ;
extern int vsetstr_count(VSETSTR *) ;
extern int vsetstr_extent(VSETSTR *) ;
extern int vsetstr_curbegin(VSETSTR *,VSETSTR_CUR *) ;
extern int vsetstr_curend(VSETSTR *,VSETSTR_CUR *) ;
extern int vsetstr_curdel(VSETSTR *,VSETSTR_CUR *) ;
extern int vsetstr_enum(VSETSTR *,VSETSTR_CUR *,cchar **) ;
extern int vsetstr_next(VSETSTR *,VSETSTR_CUR *) ;
extern int vsetstr_finish(VSETSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VSETSTR_MASTER */

#endif /* VSETSTR_INCLUDE */


