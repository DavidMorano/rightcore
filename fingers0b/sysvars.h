/* sysvars */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSVARS_INCLUDE
#define	SYSVARS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>

#include	"var.h"


#define	SYSVARS		struct sysvars_head
#define	SYSVARS_CUR	struct sysvars_c
#define	SYSVARS_OBJ	struct sysvars_obj
#define	SYSVARS_FL	struct sysvars_flags

/* object defines */
#define	SYSVARS_MAGIC	0x99889298

/* query options */
#define	SYSVARS_OPREFIX	(1<<0)		/* prefix match */


struct sysvars_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct sysvars_c {
	VAR_CUR		vcur ;
} ;

struct sysvars_flags {
	uint		var:1 ;		/* index DB is open */
} ;

struct sysvars_head {
	uint		magic ;
	const void	*a ;		/* allocation */
	const char	*pr ;
	const char	*dbname ;	/* DB name (allocated) */
	SYSVARS_FL	f ;
	VAR		vind ;		/* variable index */
	time_t		ti_db ;		/* DB mtime */
	int		ncursors ;
} ;


#if	(! defined(SYSVARS_MASTER)) || (SYSVARS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysvars_open(SYSVARS *,const char *,const char *) ;
extern int sysvars_count(SYSVARS *) ;
extern int sysvars_curbegin(SYSVARS *,SYSVARS_CUR *) ;
extern int sysvars_fetch(SYSVARS *, const char *,int,SYSVARS_CUR *,
				char *,int) ;
extern int sysvars_enum(SYSVARS *,SYSVARS_CUR *,char *,int,char *,int) ;
extern int sysvars_curend(SYSVARS *,SYSVARS_CUR *) ;
extern int sysvars_audit(SYSVARS *) ;
extern int sysvars_close(SYSVARS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSVARS_MASTER */

#endif /* SYSVARS_INCLUDE */


