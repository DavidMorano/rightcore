/* dijkstra-2 */
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

	+ close to optimal speed because we used queue for future work


	Complexity:

	O ( |v| + |v|log|e| )

	Implementation:

	We assume we have an object known as |minheap|, with methods:
	+ push()
	+ pop()
	+ descrease()
	+ top()


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
#include	<queue>
#include	<vector>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"dijkstra2.hh"


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

struct nodeval {
	int		v = 0 ;
	int		dist = 0 ;
	nodeval() { } ;
	nodeval(int av,int ad) : v(av), dist(ad) {
	} ;
	nodeval(const nodeval &other) {
	    v = other.v ;
  	    dist = other.dist ;
	} ;
	nodeval &operator = (const nodeval &other) {
	    v = other.v ;
  	    dist = other.dist ;
	    return (*this) ;
	} ;
	nodeval &operator = (initializer_list<int> &il) {
	    int	i = 0 ;
	    for (auto in : il) {
		switch (i++) {
		case 0:
		    v = in ;
		    break ;
		case 1:
		    dist = in ;
		    break ;
		} /* end switch */
	    } /* end for */
	    return (*this) ;
	} ;
} ;

struct nodevalcmp {
	bool operator () (const nodeval &nv1,const nodeval &nv2) {
	     return (nv1.dist > nv2.dist) ;
	} ;
} ;

typedef priority_queue<nodeval,vector<nodeval>,nodevalcmp>	ourmin ;


/* forward refereces */


/* exported subroutines */


int dijkstra2(res_t *resp,edges_t &edges,int vertices,int vstart)
{
	int		rs = SR_OK ;
	bool		*visited ;
	if ((visited = new(nothrow) bool [vertices+1]) != NULL) {
	    const int	ne = edges.size() ;
	    ourmin	pq ; /* min-heap */
	    nodeval	nv ;
	    edgeit_t	elit ; /* edge-list-iterator */
	    edgeit_t	end ; /* edge-list-iterator */
	    int		i ;

	    for (i = 0 ; i < vertices ; i += 1) visited[i] = false ;

	    for (i = 0 ; i < vertices ; i += 1) {
	        resp[i].dist = INT_MAX ;
	        resp[i].prev = -1 ;
	    }

	    resp[vstart].dist = 0 ;

	    nv = {vstart,0} ;
	    pq.push(nv) ;

	    for (i = 0 ; i < (vertices-1) ; i += 1) {
	        nodeval	nv = pq.top() ;
		int	u, w ;
		u = nv.v ;
		w = nv.dist ;
	        if ((u < ne) && (w < INT_MAX)) {
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
	    			nv = {v,(d+w)} ;
			        pq.push(nv) ;
	                    }

	                } /* end if (distance to current vertex not INF) */
	                elit++ ;
	            } /* end while */
	        } /* end block */
	        pq.pop() ;
	    } /* end for */

	} else {
	    rs = SR_NOMEM ;
	}
	return rs ;
}
/* end subroutine (dijkstra2) */


/* local subroutines */


