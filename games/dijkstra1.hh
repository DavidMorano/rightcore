/* dijkstra-1 */
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

#ifndef	DIJKSTRA1_INCLUDE
#define	DIJKSTRA1_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vector>
#include	<list>
#include	<localmisc.h>

#include	"graph.hh"


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif


/* local structures */


extern int dijkstra1(graph_res *,
		std::vector<std::list<graph_edge>> &,
		int,int) ;


#endif /* DIJKSTRA1_INCLUDE */


