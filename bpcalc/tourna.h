/* tourna */


#ifndef	TOURNA_INCLUDE
#define	TOURNA_INCLUDE	1


#include	<sys/types.h>
#include	<time.h>

#include	"localmisc.h"



/* object defines */

#define	TOURNA			struct tourna_head
#define	TOURNA_STATS		struct tourna_stats






/* statistics */
struct tourna_stats {
	uint			gpht ;		/* global length */
	uint			lbht ;		/* local history length */
	uint			lpht ;		/* local pattern length */
	uint			bits ;
} ;

struct tourna_head {
	unsigned long		magic ;
	struct tourna_stats	s ;
	uint			*lbht ;		/* local BHT */
	uchar			*lpht ;		/* local PHT */
	uchar			*cpht ;		/* choice PHT */
	uchar			*gpht ;		/* global PHT */
	uint			bhistory ;	/* global branch history */
	uint			lhlen ;		/* local history length */
	uint			lplen ;		/* local pattern length */
	uint			glen ;		/* global length */
	uint			historymask ;
} ;



#if	(! defined(TOURNA_MASTER)) || (TOURNA_MASTER == 0)

extern int	tourna_init(TOURNA *,int,int,int) ;
extern int	tourna_lookup(TOURNA *,uint) ;
extern int	tourna_confidence(TOURNA *,uint) ;
extern int	tourna_update(TOURNA *,uint,int) ;
extern int	tourna_zerostats(TOURNA *) ;
extern int	tourna_stats(TOURNA *,TOURNA_STATS *) ;
extern int	tourna_free(TOURNA *) ;

#endif /* TOURNA_MASTER */


#endif /* TOURNA_INCLUDE */



