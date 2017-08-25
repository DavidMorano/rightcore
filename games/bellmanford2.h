/* bellmanford-2 */
/* lang=C++11 */

/* Bellman-Ford algorithm for shortest path through graph */


/* revision history:

	= 2013-03-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Bellman-Ford (using array of edges)


*******************************************************************************/

#ifndef	BELLMANFORD2_INCLUDE
#define	BELLMANFORD2_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vector>
#include	<list>
#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif


/* local structures */

struct bellmanford2_edge {
	int		src ;	/* source vertex */
	int		dst ;	/* destination vertex */
	int		weight ; /* weight of edge to this vertex */
} ;

struct bellmanford2_res {
	int		prev ; /* previous vertex */
	int		dist ; /* distance (summed weight) to present vertex */
} ;


extern int bellmanford2(bellmanford2_res *,
	        struct bellmanford2_edge *,
		int,int) ;


#endif /* BELLMANFORD2_INCLUDE */


