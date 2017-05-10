/* gskew */


#ifndef	GSKEW_INCLUDE
#define	GSKEW_INCLUDE	1


#include	<sys/types.h>
#include	<time.h>

#include	"localmisc.h"



/* object defines */

#define	GSKEW			struct gskew_head
#define	GSKEW_STATS		struct gskew_stats


/* more important defines */

#define	GSKEW_COUNTBITS	2	/* counter bits */




/* statistics */
struct gskew_stats {
	uint	tlen ;
	uint	bits ;
	uint	lu ;
	uint	use_bim ;
	uint	use_eskew ;
	uint	update_meta ;
	uint	updateup_meta ;
	uint	update_all, update_bim, update_eskew ;
} ;

struct gskew_banks {
	uint	bim : 2 ;
	uint	g0 : 2 ;
	uint	g1 : 2 ;
	uint	meta : 2 ;
} ;

struct gskew_head {
	unsigned long		magic ;
	struct gskew_stats	s ;
	struct gskew_banks	*table ;
	uint			bhistory ;	/* global branch history */
	uint			tlen ;
	int			n ;		/* 'n' from the papers ! :-) */
	uint			nhist ;		/* history bits */
	uint			tmask, hmask ;
} ;



#if	(! defined(GSKEW_MASTER)) || (GSKEW_MASTER == 0)

extern int	gskew_init(GSKEW *,int,int,int,int) ;
extern int	gskew_lookup(GSKEW *,uint) ;
extern int	gskew_confidence(GSKEW *,uint) ;
extern int	gskew_update(GSKEW *,uint,int) ;
extern int	gskew_zerostats(GSKEW *) ;
extern int	gskew_stats(GSKEW *,GSKEW_STATS *) ;
extern int	gskew_free(GSKEW *) ;

#endif /* GSKEW_MASTER */


#endif /* GSKEW_INCLUDE */



