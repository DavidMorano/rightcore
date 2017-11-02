/* graph */
/* lang=C++11 */

/* Dijkstra (shortest path through graph) */


/* revision history:

	= 2013-03-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We collect some graph-related elements.


*******************************************************************************/

#ifndef	GRAPH_INCLUDE
#define	GRAPH_INCLUDE	1

#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vector>
#include	<list>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif


/* local structures */

struct graph_edger {
	int		src ;	/* source vertex */
	int		dst ;	/* destination vertex */
	int		weight ; /* weight of edge to this vertex */
} ;

struct graph_edge {
	int		dst ;	/* destination vertex */
	int		weight ; /* weight of edge to this vertex */
	graph_edge() : dst(0), weight(0) { } ;
	graph_edge(int adst,int aweight = 0) : dst(adst), weight(aweight) {
	} ;
} ;

struct graph_res {
	int		dist ; /* distance (summed weight) to present vertex */
	int		prev ; /* previous vertex */
	graph_res() : dist(0), prev(-1) { } ;
	graph_res(int adist,int aprev = -1) : dist(adist), prev(aprev) {
	} ;
	graph_res &operator = (const graph_res &other) {
	    dist = other.dist ;
	    prev = other.prev ;
	    return (*this) ;
	} ;
	graph_res &operator = (int adist) {
	    dist = adist ;
	    prev = -1 ;
	    return (*this) ;
	} ;
} ;


#endif /* GRAPH_INCLUDE */


