/* smservices */


#ifndef	SMSERVICES_INCLUDE
#define	SMSERVICES_INCLUDE



/* object defines */

#define	SMSERVICES		struct smservices_head
#define	SMSERVICES_CUR	struct smservices_c
#define	SMSERVICES_ENT	struct smservices_e


#define	SMSERVICES_SVCLEN	32



#include	<sys/types.h>

#include	<vecelem.h>

#include	"localmisc.h"




struct smservices_head {
	unsigned long	magic ;
	vecelem		entries ;
	char		*fname ;
} ;

struct smservices_e {
	uint	svc ;
	char	canonname[SMSERVICES_SVCLEN + 1] ;
	char	svcname[SMSERVICES_SVCLEN + 1] ;
} ;

struct smservices_c {
	int	i ;
} ;




#if	(! defined(SMSERVICES_MASTER)) || (SMSERVICES_MASTER == 0)

extern int smservices_open(SMSERVICES *,char *) ;
extern int smservices_close(SMSERVICES *) ;
extern int smservices_curbegin(SMSERVICES *,SMSERVICES_CUR *) ;
extern int smservices_curend(SMSERVICES *,SMSERVICES_CUR *) ;
extern int smservices_fetch(SMSERVICES *,SMSERVICES_CUR *,
		SMSERVICES_ENT *) ;

#endif /* SMSERVICES_MASTER */


#endif /* SMSERVICES_INCLUDE */



