/* pcsconfs */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	PCSCONFS_INCLUDE
#define	PCSCONFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>

#include	"var.h"


#define	PCSCONFS	struct pcsconfs_head
#define	PCSCONFS_CUR	struct pcsconfs_c
#define	PCSCONFS_OBJ	struct pcsconfs_obj

/* query options */

#define	PCSCONFS_OPREFIX	0x01		/* prefix match */


struct pcsconfs_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct pcsconfs_c {
	VAR_CUR		vcur ;
} ;

struct pcsconfs_flags {
	uint		prdb:1 ;		/* CONF is global */
	uint		db:1 ;			/* DB is open */
} ;

struct pcsconfs_head {
	uint		magic ;
	const char	**envv ;
	const char	*pr ;			/* program-root */
	const char	*cfname ;		/* DB database name */
	const char	*a ;			/* memory allocation */
	struct pcsconfs_flags	f ;
	VAR		db ;
	time_t		ti_conf ;		/* DB mtime */
	int		ncursors ;
} ;


#if	(! defined(PCSCONFS_MASTER)) || (PCSCONFS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsconfs_start(PCSCONFS *,const char *,const char **,const char *) ;
extern int pcsconfs_curbegin(PCSCONFS *,PCSCONFS_CUR *) ;
extern int pcsconfs_fetch(PCSCONFS *, const char *,int,PCSCONFS_CUR *,
				char *,int) ;
extern int pcsconfs_enum(PCSCONFS *,PCSCONFS_CUR *,char *,int,char *,int) ;
extern int pcsconfs_curend(PCSCONFS *,PCSCONFS_CUR *) ;
extern int pcsconfs_audit(PCSCONFS *) ;
extern int pcsconfs_finish(PCSCONFS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSCONFS_MASTER */


#endif /* PCSCONFS_INCLUDE */



