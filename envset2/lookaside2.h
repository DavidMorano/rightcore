/* lookaside */

/* lookaside memory allocation manager */


#ifndef	LOOKASIDE_INCLUDE
#define	LOOKASIDE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<pq.h>

#include	"localmisc.h"



/* object defines */

#define	LOOKASIDE		struct lookaside_head



struct lookaside_head {
	caddr_t		eap ;		/* entry allocation pointer base */
	PQ		cq ;		/* chunk list */
	PQ		estack ;	/* stack of free blocks */
	int		nchunks ;	/* number of chunks allocated */
	int		esize ;		/* entry size */
	int		eaoff ;		/* entry-array offset */
	int		n ;		/* entries per chunk */
	int		i ;		/* current chunk usage (index) */
	int		nfree ;		/* total free entries */
	int		nused ;		/* total entries used */
} ;



#if	(! defined(LOOKASIDE_MASTER)) || (LOOKASIDE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int lookaside_init(LOOKASIDE *,int,int) ;
extern int lookaside_get(LOOKASIDE *,void *) ;
extern int lookaside_release(LOOKASIDE *,void *) ;
extern int lookaside_count(LOOKASIDE *) ;
extern int lookaside_free(LOOKASIDE *) ;
extern int lookaside_audit(LOOKASIDE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(LOOKASIDE_MASTER)) || (LOOKASIDE_MASTER == 0) */


#endif /* LOOKASIDE_INCLUDE */



