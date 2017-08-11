/* dijkstra-2 */
/* lang=C++11 */

/* Dijkstra (shortest path through graph) */


/* revision history:

	= 2013-03-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Dijkstra-1 (shorted path through graph)


*******************************************************************************/

#ifndef	DIJKSTRA2_INCLUDE
#define	DIJKSTRA2_INCLUDE	1


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

struct dijkstra2_edge {
	int		dst ;	/* destination vertex */
	int		weight ; /* weight of edge to this vertex */
} ;

struct dijkstra2_res {
	int		prev ; /* previous vertex */
	int		dist ; /* distance (summed weight) to present vertex */
} ;


extern int dijkstra2(dijkstra2_res *,
		std::vector<std::list<dijkstra2_edge>> *,
		int,int) ;


#endif /* DIJKSTRA2_INCLUDE */


