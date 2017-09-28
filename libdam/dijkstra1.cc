/* dijkstra-1 */
/* lang=C++11 */

/* Dijkstra (shortest path through graph) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-03-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This executes the Dijjstra algorithm to find the sorted path through a
        weighted graph.


	Fatures:

	+ pretty close to optimal speed (no queue, just linear search)


	Complexity:

	O ( |v| + |v|log|e| )


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<climits>
#include	<cinttypes>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<vector>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"dijkstra1.hh"


/* local defines */


/* local namespaces */

using namespace	std ;


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif


/* local structures */

typedef graph_res		res_t ;
typedef graph_edge		edge_t ;

typedef vector<list<edge_t>>	edges_t ;
typedef list<edge_t>::iterator	edgeit_t ;


/* forward refereces */

static int minvertex(bool *,res_t *,int) ;


/* exported subroutines */


int dijkstra1(res_t *resp,edges_t &edges,int vertices,int vstart)
{
	int		rs = SR_OK ;
	bool		*visited ;
	if ((visited = new(nothrow) bool [vertices+1]) != NULL) {
	    edgeit_t	elit ; /* edge-list-iterator */
	    edgeit_t	end ; /* edge-list-iterator */
	    const int	ne = edges.size() ;
	    int		i ;

	    for (i = 0 ; i < vertices ; i += 1) visited[i] = false ;

	    for (i = 0 ; i < vertices ; i += 1) {
	        resp[i].dist = INT_MAX ;
	        resp[i].prev = -1 ;
	    }

	    resp[vstart].dist = 0 ;

	    for (i = 0 ; i < (vertices-1) ; i += 1) {
	        const int	u = minvertex(visited,resp,vertices) ;
	        if ((u >= 0) && (u < ne)) {
		    cout << "u=go" << endl ;
	            elit = edges[u].begin() ; /* this is 'list.begin()' */
	            end = edges[u].end() ; /* this is 'list.end()' */

		    visited[u] = true ;
	            while (elit != end) {
	                const int	v = (*elit).dst ; /* dst vertex */
	                if ((! visited[v]) && (resp[u].dist != INT_MAX)) {
	                    const int	d = resp[u].dist ;
	                    const int	w = (*elit).weight ;

	                    if ((d+w) < resp[v].dist) {
	                        resp[v].dist = (d+w) ;
	                        resp[v].prev = u ;
	                    }

	                } /* end if (distance to current vertex not INF) */
	                elit++ ;
	            } /* end while */

	        } /* end block */
	    } /* end for */

	} else {
	    rs = SR_NOMEM ;
	}
	return rs ;
}
/* end subroutine (dijkstra1) */


/* local subroutines */


static int minvertex(bool *visited,res_t *resp,int n)
{
	int	min = INT_MAX ;
	int	v = -1 ;
	int	i ;
   	for (i = 0 ; i < n ; i += 1) {
            if (! visited[i]) {
		if (resp[i].dist <= min) {
		    min = resp[i].dist ;
		    v = i ;
		}
	    }
	}
   	return v ;
}
/* end subroutine (minvertex) */


