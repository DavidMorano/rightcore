/* dbi */


#ifndef	DBI_INCLUDE
#define	DBI_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<nodedb.h>
#include	<clusterdb.h>
#include	<vecstr.h>
#include	<localmisc.h>


#define	DBI		struct dbi
#define	DBI_FL		struct dbi_flags


struct dbi_flags {
	uint		node:1 ;
	uint		cluster:1 ;
} ;

struct dbi {
	NODEDB		node ;
	CLUSTERDB	cluster ;
	DBI_FL		open ;
} ;


#if	(! defined(DBI_MASTER)) || (DBI_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dbi_open(DBI *,cchar *) ;
extern int dbi_getclusters(DBI *,vecstr *,cchar *) ;
extern int dbi_getnodes(DBI *,vecstr *,vecstr *) ;
extern int dbi_close(DBI *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DBI_MASTER */


#endif /* DBI_INCLUDE */


