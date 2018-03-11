/* bfs-1 */
/* lang=C++11 */

/* Breath-First-Search (shortest route through un-weighted graph) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-03-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the (famous) Breath-First-Search algorithm to find the shorted
        (least number of hops) route through an un-weighted graph.

	Complexity:

	O ( |v| + |e| )

	Synopsis:

	int bfs1(res_t *resp,edges_t &edges,int vertices,int vstart)

	Arguments:

	resp		array of result structures:
				dist	level or distance from starting node
				prev	previous node (ancestor)
	edges		vector of lists describing adjacency edges of graph
	vertices	number of vertices in graph
	vstart		starting vertex

	Returns:

	-		result structs are filled in 


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<limits.h>
#include	<cinttypes>
#include	<new>
#include	<algorithm>
#include	<functional>
#include	<list>
#include	<vector>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfs1.hh"


/* local defines */


/* local namespaces */

using namespace	std ;


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif


/* local structures */

typedef bfs1_res		res_t ;
typedef bfs1_edge		edge_t ;
typedef vector<list<edge_t>>	edges_t ;
typedef list<edge_t>::iterator	edgeit_t ;


/* forward refereces */


/* exported subroutines */


int bfs1(res_t *resp,edges_t &edges,int vertices,int vstart)
{
	list<int>	vq ; /* vertex-queue */
	int		rs = SR_OK ;
	edgeit_t	elit ; /* edge-list-iterator */
	edgeit_t	end ; /* edge-list-iterator */
	int		i ;

	for (i = 0 ; i < vertices ; i += 1) {
	    resp[i].dist = -1 ; /* "level" */
	    resp[i].prev = -1 ;
	}

	resp[vstart].dist = 0 ; /* "level" */
	resp[vstart].prev = -1 ;

	vq.push_back(vstart) ;

	while (! vq.empty()) {
	    const int	u = vq.front() ;
	    {
	        elit = edges[u].begin() ; /* this is 'list.begin()' */
	        end = edges[u].end() ; /* this is 'list.end()' */
	        while (elit != end) {
	            const int	v = (*elit).dst ; /* dst vertex */
	            if (resp[v].dist < 0) {
	                resp[v].dist = (resp[u].dist + 1) ;
	                resp[v].prev = u ;
			vq.push_back(v) ;
	            } /* end if */
	            elit++ ;
		} /* end while */
	    } /* end block */
	    vq.pop_front() ;
	} /* end while */

	return rs ;
}
/* end subroutine (bfs1) */


