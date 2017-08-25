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
#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif


/* local structures */

struct dijkstra1_edge {
	int		dst ;	/* destination vertex */
	int		weight ; /* weight of edge to this vertex */
	dijkstra1_edge() : dst(0), weight(0) { } ;
	dijkstra1_edge(int adst,int aweight = 0) : dst(adst), weight(aweight) {
	} ;
} ;

struct dijkstra1_res {
	int		dist ; /* distance (summed weight) to present vertex */
	int		prev ; /* previous vertex */
	dijkstra1_res() : dist(0), prev(-1) { } ;
	dijkstra1_res(int adist,int aprev = -1) : dist(adist), prev(aprev) {
	} ;
	dijkstra1_res &operator = (const dijkstra1_res &other) {
	    dist = other.dist ;
	    prev = other.prev ;
	    return (*this) ;
	} ;
	dijkstra1_res &operator = (int adist) {
	    dist = adist ;
	    prev = -1 ;
	    return (*this) ;
	} ;
} ;


extern int dijkstra1(dijkstra1_res *,
		std::vector<std::list<dijkstra1_edge>> &,
		int,int) ;


#endif /* DIJKSTRA1_INCLUDE */


