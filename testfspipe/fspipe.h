/* fspipe */


#ifndef	FSPIPE_INCLUDE
#define	FSPIPE_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecelem.h>

#include	"localmisc.h"



/* object defines */

#define	FSPIPE			struct fspipe_head
#define	FSPIPE_ENT		struct fspipe_e

#define	FSPIPE_VERSION		0
#define	FSPIPE_TRANSLEN		32
#define	FSPIPE_PROTOLEN		32
#define	FSPIPE_MAGICSTR		"FSPIPE"
#define	FSPIPE_MAGICSTRLEN	6



struct fspipe_flags {
	uint	listen : 1 ;		/* listen mode */
} ;

struct fspipe_tunix {
	char	*nodename ;
	char	*filepath ;
} ;

struct fspipe_tinet {
	char	*hostname ;
	char	*portspec ;
} ;

struct fspipe_tlocal {
	char	*nodename ;
	char	*filepath ;		/* server pipe or FIFO */
	char	*file2path ;		/* client FIFO if used */
} ;

union fspipe_param {
	struct fspipe_tlocal	t_local ;
	struct fspipe_tunix	t_unix ;
	struct fspipe_tinet	t_inet ;
} ;

struct fspipe_e {
	union fspipe_param	p ;
	int	type ;
	int	fd ;
	char	transport[FSPIPE_TRANSLEN + 1] ;
	char	protocol[FSPIPE_PROTOLEN + 1] ;
} ;

struct fspipe_head {
	ulong		magic ;
	struct fspipe_flags	f ;
	vecelem		e ;
	int		version ;
	int		fd ;
} ;


#ifndef	FSPIPE_MASTER


#endif /* FSPIPE_MASTER */


#endif /* FSPIPE_INCLUDE */



