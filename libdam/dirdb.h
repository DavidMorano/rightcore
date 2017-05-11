/* dirdb */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DIRDB_INCLUDE
#define	DIRDB_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<vechand.h>
#include	<hdb.h>
#include	<localmisc.h>


#define	DIRDB_MAGIC	0x33119572
#define	DIRDB_NDEF	30
#define	DIRDB		struct dirdb_head
#define	DIRDB_ENT	struct dirdb_e
#define	DIRDB_FID	struct dirdb_fid
#define	DIRDB_CUR	struct dirdb_c


struct dirdb_fid {
	uino_t		ino ;
	dev_t		dev ;
} ;

struct dirdb_e {
	const char	*name ;
	DIRDB_FID	fid ;
	int		count ;
} ;

struct dirdb_head {
	uint		magic ;
	VECHAND		dlist ;
	HDB		db ;
	int		count ;
} ;

struct dirdb_c {
	int		i ;
} ;


#if	(! defined(DIRDB_MASTER)) || (DIRDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dirdb_start(DIRDB *,int) ;
extern int dirdb_add(DIRDB *,const char *,int) ;
extern int dirdb_clean(DIRDB *) ;
extern int dirdb_curbegin(DIRDB *,DIRDB_CUR *) ;
extern int dirdb_enum(DIRDB *,DIRDB_CUR *,DIRDB_ENT **) ;
extern int dirdb_curend(DIRDB *,DIRDB_CUR *) ;
extern int dirdb_finish(DIRDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(DIRDB_MASTER)) || (DIRDB_MASTER == 0) */

#endif /* DIRDB_INCLUDE */


