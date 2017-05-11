/* clusterdb */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CLUSTERDB_INCLUDE
#define	CLUSTERDB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<netdb.h>

#include	<kvsfile.h>
#include	<localmisc.h>


/* object defines */

#define	CLUSTERDB	struct clusterdb_head
#define	CLUSTERDB_CUR	struct clusterdb_c


struct clusterdb_c {
	KVSFILE_CUR	cur ;
} ;

struct clusterdb_head {
	uint		magic ;
	KVSFILE		clutab ;
} ;


#if	(! defined(CLUSTERDB_MASTER)) || (CLUSTERDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int clusterdb_open(CLUSTERDB *,const char *) ;
extern int clusterdb_fileadd(CLUSTERDB *,const char *) ;
extern int clusterdb_curbegin(CLUSTERDB *,CLUSTERDB_CUR *) ;
extern int clusterdb_curend(CLUSTERDB *,CLUSTERDB_CUR *) ;
extern int clusterdb_enumcluster(CLUSTERDB *,CLUSTERDB_CUR *,
		char *,int) ;
extern int clusterdb_enum(CLUSTERDB *,CLUSTERDB_CUR *,
		char *,int,char *,int) ;
extern int clusterdb_fetch(CLUSTERDB *,const char *,CLUSTERDB_CUR *,
		char *,int) ;
extern int clusterdb_fetchrev(CLUSTERDB *,const char *,CLUSTERDB_CUR *,
		char *,int) ;
extern int clusterdb_check(CLUSTERDB *,time_t) ;
extern int clusterdb_close(CLUSTERDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CLUSTERDB_MASTER */

#endif /* CLUSTERDB_INCLUDE */


