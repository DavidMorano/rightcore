/* srvpe */


#ifndef	SRVPE_INCLUDE
#define	SRVPE_INCLUDE	1



#include	<sys/types.h>
#include	<sys/param.h>
#include	<netdb.h>

#include	"srvtab.h"

#include	"srventry.h"


/* object defines */

#define	SRVPE		struct srvpe_static
#define	SRVPE_ARGS	struct srvpe_dynamic





struct srvpe_static {
	char	*version ;		/* %V */
	char	*programroot ;		/* %R */
	char	*nodename ;		/* %N */
	char	*domainname ;		/* %D */
	char	*hostname ;		/* %H (created and allocated) */
} ;

struct srvpe_dynamic {
	char	*service ;
	char	*srvargs ;
	char	*peername ;
	char	*nethost ;
	char	*netuser ;
	char	*netpass ;
} ;




#ifndef	SRVPE_MASTER

extern int srvpe_init(SRVPE *,char *,char *,char *,char *) ;
extern int srvpe_free(SRVPE *) ;
extern int srvpe_sub(SRVPE *,SRVPE_ARGS *,SRVTAB_ENT *,SRVENTRY *) ;

#endif /* SRVPE_MASTER */


#endif /* SRVPE_INCLUDE */



