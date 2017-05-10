/* yags */


#ifndef	YAGS_INCLUDE
#define	YAGS_INCLUDE	1


#include	<sys/types.h>
#include	<time.h>

#include	"localmisc.h"
#include	"bpload.h"



/* object defines */

#define	YAGS			struct yags_head
#define	YAGS_STATS		struct yags_stats


/* more important defines */

#define	YAGS_TAGBITS		8	/* number of tag bits */
#define	YAGS_COUNTBITS		2	/* counter bits */



#define	YAGS_TAGMASK		((1 << YAGS_TAGBITS) - 1)



/* statistics */
struct yags_stats {
	uint			cpht ;		/* choice length */
	uint			dpht ;		/* cache length */
	uint			bits ;		/* total bits */
} ;

struct yags_pht {
	uint	counter : 2 ;
} ;

struct yags_cache {
	uint	tag0 : (YAGS_TAGBITS + 1) ;	/* IAoff + 1 history */
	uint	tag1 : (YAGS_TAGBITS + 1) ;	/* IAoff + 1 history */
	uint	counter0 : 2 ;
	uint	counter1 : 2 ;
	uint	lru : 1 ;			/* least recently used */
} ;

struct yags_head {
	unsigned long		magic ;
	struct yags_stats	s ;
	struct yags_pht		*choice ;
	struct yags_cache	*taken, *nottaken ;
	uint			bhistory ;	/* global branch history */
	uint			chlen ;		/* choice length */
	uint			calen ;		/* cache length */
} ;



#if	(! defined(YAGS_MASTER)) || (YAGS_MASTER == 0)

extern int	yags_init(YAGS *,int,int) ;
extern int	yags_lookup(YAGS *,uint) ;
extern int	yags_update(YAGS *,uint,int) ;
extern int	yags_zerostats(YAGS *) ;
extern int	yags_stats(YAGS *,YAGS_STATS *) ;
extern int	yags_free(YAGS *) ;

#endif /* YAGS_MASTER */


#endif /* YAGS_INCLUDE */



