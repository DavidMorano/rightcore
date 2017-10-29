/* dw */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DW_INCLUDE
#define	DW_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>

#include	<vecobj.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* defines */

#define	DW_MAGIC	0x31415926
#define	DW		struct dw_head
#define	DW_ENT		struct dw_ent
#define	DW_CUR		struct dw_cur

#define	DW_DEFCHECKTIME	20		/* default check time (seconds) */


struct dw_ent {
	time_t		itime ;
	time_t		mtime ;
	size_t		size ;
	int		state ;
	char		name[MAXPATHLEN + 1] ;
} ;

struct dw_ientry {
	const char	*name ;
	time_t		itime ;
	time_t		mtime ;
	size_t		size ;
	int		state ;
} ;

struct dw_flags {
	uint		subdirs:1 ;
} ;

struct dw_head {
	uint		magic ;
	struct dw_flags	f ;
	VECSTR		subdirs ;		/* subdirectories */
	VECOBJ		e ;			/* directory entries */
	const char	*dirname ;		/* directory path */
	void		(*callback)(DW_ENT *,int,void *) ;
	const void	*argp ;
	time_t		opentime ;		/* time FD was cached */
	time_t		mtime ;			/* directory mod-time */
	time_t		checktime ;		/* time last checked */
	time_t		removetime ;		/* last checked for removed */
	int		checkinterval ;		/* file check interval */
	int		fd ;			/* cached directory FD */
	int		count_new ;
	int		count_checkable ;
} ;

struct dw_cur {
	int		i ;
} ;


/* job states */

#define	DW_SRESERVED	0
#define	DW_SNEW		1	/* just in */
#define	DW_SCHECK	2	/* ready for validation check */
#define	DW_SREADY	3
#define	DW_SNOTSUP	4
#define	DW_SRUNNING	5
#define	DW_SRETRY	6
#define	DW_SUSER	10


#if	(! defined(DW_MASTER)) || (DW_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dw_start(DW *,const char *) ;
extern int dw_finish(DW *) ;
extern int dw_find(DW *,const char *,DW_ENT *) ;
extern int dw_del(DW *,DW_CUR *) ;
extern int dw_check(DW *,time_t) ;
extern int dw_curbegin(DW *,DW_CUR *) ;
extern int dw_curend(DW *,DW_CUR *) ;
extern int dw_enum(DW *,DW_CUR *,DW_ENT *) ;
extern int dw_enumcheckable(DW *,DW_CUR *,DW_ENT *) ;
extern int dw_state(DW *,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* DW_MASTER */

#endif /* DW_INCLUDE */


