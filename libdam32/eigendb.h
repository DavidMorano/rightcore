/* eigendb */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	EIGENDB_INCLUDE
#define	EIGENDB_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<hdb.h>
#include	<strpack.h>

#include	<localmisc.h>


#define	EIGENDB		struct eigendb_head
#define	EIGENDB_CUR	HDB_CUR
#define	EIGENDB_MAGIC	0x83726112


struct eigendb_head {
	uint		magic ;
	STRPACK		packer ;
	HDB		db ;
} ;


#if	(! defined(EIGENDB_MASTER)) || (EIGENDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	eigendb_open(EIGENDB *,const char *) ;
extern int	eigendb_addfile(EIGENDB *,const char *) ;
extern int	eigendb_addword(EIGENDB *,const char *,int) ;
extern int	eigendb_exists(EIGENDB *,const char *,int) ;
extern int	eigendb_count(EIGENDB *) ;
extern int	eigendb_curbegin(EIGENDB *,EIGENDB_CUR *) ;
extern int	eigendb_curend(EIGENDB *,EIGENDB_CUR *) ;
extern int	eigendb_enum(EIGENDB *,EIGENDB_CUR *,const char **) ;
extern int	eigendb_close(EIGENDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* EIGENDB_MASTER */

#endif /* EIGENDB_INCLUDE */


