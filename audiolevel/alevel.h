/* alevel */

/* audio level object */


#ifndef	ALEVEL_INCLUDE
#define	ALEVEL_INCLUDE	1



#include	<sys/types.h>

#include	"localmisc.h"



/* object defines */

#define	ALEVEL		struct alevel_head




struct alevel_head {
	ULONG	hits[100] ;	/* level hits */
	ULONG	c ;		/* count of samples */
} ;



#if	(! defined(ALEVEL_MASTER)) || (ALEVEL_MASTER == 0)

extern int alevel_init(ALEVEL *) ;
extern int alevel_zero(ALEVEL *) ;
extern int alevel_proc(ALEVEL *,float *,int) ;
extern int alevel_getlevel(ALEVEL *,ULONG *) ;
extern int alevel_free(ALEVEL *) ;

#endif /* ALEVEL_MASTER */


#endif /* ALEVEL_INCLUDE */



