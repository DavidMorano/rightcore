/* ddb (unneeded, unfinished) */

/* domain data-base */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DDB_INCLUDE
#define	DDB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<vecstr.h>
#include	<hdb.h>
#include	<localmisc.h>


/* object defines */

#define	DDB		struct ddb_head


struct ddb_head {
	const char	*fname ;
	vecstr		keys ;
	HDB		db ;
	time_t		mtime ;
} ;


#if	(! defined(DDB_MASTER)) || (DDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ddb_open(DDB *,const char *,const char *) ;
extern int ddb_search(DDB *,const char *,char *) ;
extern int ddb_close(DDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DDB_MASTER */

#endif /* DDB_INCLUDE */


