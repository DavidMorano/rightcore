/* bellmanford-1 */
/* lang=C++11 */

/* Bellman-Ford algorithm for shortest path through graph */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-03-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is better than Dijjstra because this algorithm can handle negative
        edge weights and will detect an overall negative path length.

	Complexity:

	time worst	O ( |v| · |e| )
	time best	O ( |e| )
	space		O ( |v| )


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
#include	<vsystem.h>
#include	<localmisc.h>

#include	"bellmanford1.hh"


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


/* forward references */


/* exported subroutines */


int bellmanford1(res_t *resp,edges_t &edges,int vertices,int vstart)
{
	edgeit_t	elit ; /* edge-list-iterator */
	edgeit_t	end ; /* edge-list-iterator */
	const int	ne = edges.size() ;
	int		rs = SR_OK ;
	int		i ;
	int		u ;

	for (i = 0 ; i < vertices ; i += 1) {
	    resp[i].dist = INT_MAX ;
	    resp[i].prev = -1 ;
	}

	resp[vstart].dist = 0 ;

	for (i = 0 ; i < (vertices-1) ; i += 1) {
	    int		f_nochange = TRUE ;
	    for (u = 0 ; u < ne ; u += 1) { /* edges(u,v) */
	        elit = edges[u].begin() ; /* this is 'list.begin()' */
	        end = edges[u].end() ; /* this is 'list.end()' */

	        while (elit != end) {
	            if (resp[u].dist != INT_MAX) {
	                const int	d = resp[u].dist ;
	                const int	w = (*elit).weight ;
	                const int	v = (*elit).dst ; /* dst vertex */

	                if ((d+w) < resp[v].dist) {
	                    resp[v].dist = (d+w) ;
	                    resp[v].prev = u ;
			    f_nochange = FALSE ;
	                }

	            } /* end if (distance to current vertex not INF) */
	            elit++ ;
	        } /* end while */
	    } /* end for */
	    if (f_nochange) break ;
	} /* end for */

/* this is the famous "extra cycle" to check for negative paths */

	for (u = 0 ; u < ne ; u += 1) {
	    const int	d = resp[u].dist ;
	    const int	v = (*elit).dst ; /* dst vertex */
	    const int	w = (*elit).weight ;
	    if ((d+w) < resp[v].dist) {
	        rs = SR_LOOP ;
	    }
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (bellmanford1) */


