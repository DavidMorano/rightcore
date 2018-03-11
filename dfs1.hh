/* dfs-1 */
/* lang=C++11 */

/* Depth-First-Search (visit all nodes through un-weighted graph) */


/* revision history:

	= 2013-03-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We check for discoverability of nodes in a graph.


*******************************************************************************/

#ifndef	DFS1_INCLUDE
#define	DFS1_INCLUDE	1


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

struct dfs1_edge {
	int		dst ;	/* destination vertex */
	int		weight ; /* weight of edge to this vertex */
} ;

struct dfs1_res {
	int		prev ; /* previous vertex */
	int		dist ; /* distance (summed weight) to present vertex */
} ;


extern int dfs1(dfs1_res *,
		std::vector<std::list<dfs1_edge>> *,
		int,int) ;


#endif /* DFS1_INCLUDE */


