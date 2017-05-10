/* dbi */


#ifndef	DBI_INCLUDE
#define	DBI_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<netdb.h>

#include	<vecstr.h>
#include	<ids.h>
#include	<nodedb.h>
#include	<clusterdb.h>
#include	<localmisc.h>


#define	DBI		struct dbi


struct dbi {
	IDS		id ;
	NODEDB		node ;
	CLUSTERDB	cluster ;
	uint		f_node : 1 ;
	uint		f_cluster : 1 ;
} ;


#if	(! defined(DBI_MASTER)) || (DBI_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dbi_open(DBI *,IDS *,const char *) ;
extern int dbi_getclusters(DBI *,vecstr *,const char *) ;
extern int dbi_getnodes(DBI *,vecstr *,vecstr *) ;
extern int dbi_close(DBI *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DBI_MASTER */

#endif /* DBI_INCLUDE */


