/* dfs-2 */
/* lang=C++11 */

/* Depth-First-Search (visit all nodes through un-weighted graph) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-03-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the (famous) Depth-First-Search algorithm to find the
        connectivity route through an un-weighted graph.

	Features:

	non-recursive (iterative)


	Complexity:

	O ( |v| + |e| )


	Synopsis:

	int dfs1(res_t *resp,edges_t &edges,int vertices)

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
#include	<stack>
#include	<vector>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"dfs2.hh"


/* local defines */


/* local namespaces */

using namespace	std ;


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif


/* local structures */

typedef dfs2_res		res_t ;
typedef dfs2_edge		edge_t ;
typedef vector<list<edge_t>>	edges_t ;
typedef list<edge_t>::iterator	edgeit_t ;


/* forward refereces */

int dfs2_visit(res_t *,edges_t &,int) ;


/* exported subroutines */


int dfs2(res_t *resp,edges_t &edges,int vertices)
{
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; i < vertices ; i += 1) {
	    resp[i].dist = -1 ; /* "level" */
	    resp[i].prev = -1 ;
	}

	for (i = 0 ; i < vertices ; i += 1) {
	    resp[i].dist = 0 ; /* level */
	    rs = dfs2_visit(resp,edges,i) ;
	    if (rs < 0) break ;
	}

	return rs ;
}
/* end subroutine (dfs2) */


int dfs2_one(res_t *resp,edges_t &edges,int vertices,int u)
{
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; i < vertices ; i += 1) {
	    resp[i].dist = -1 ; /* "level" */
	    resp[i].prev = -1 ;
	}

	resp[u].dist = 0 ; /* level */
	rs = dfs2_visit(resp,edges,u) ;

	return rs ;
}
/* end subroutine (dfs2_one) */


int dfs2_visit(res_t *resp,edges_t &edges,int u)
{
	stack<int>	vs ;
	edgeit_t	elit ; /* edge-list-iterator */
	edgeit_t	end ; /* edge-list-iterator */
	vs.push(u) ;
	while (! vs.empty()) {
	    u = vs.top() ; 
	    vs.pop() ;
	    elit = edges[u].begin() ; /* this is 'list.begin()' */
	    end = edges[u].end() ; /* this is 'list.end()' */
	    while (elit != end) {
	        const int	v = (*elit).dst ; /* dst vertex */
	        if (resp[v].dist < 0) { /* not visited */
	            resp[v].dist = (resp[u].dist + 1) ;
	            resp[v].prev = u ;
	            vs.push(v) ;
	        } /* end if */
	        elit++ ;
	    } /* end while */
	} /* end while */
	return 0 ;
}
/* end subroutine (dfs2_visit) */


