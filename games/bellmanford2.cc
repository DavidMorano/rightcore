/* bellmanford-2 */
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

#include	"bellmanford2.h"


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
typedef graph_edger		edger_t ;


/* forward references */


/* exported subroutines */


int bellmanford2(res_t *resp,edger_t *edges,int vertices,int vstart)
{
	int		rs = SR_OK ;
	int		i ;
	int		j ;

	for (i = 0 ; i < vertices ; i += 1) {
	    resp[i].dist = INT_MAX ;
	    resp[i].prev = -1 ;
	}

	resp[vstart].dist = 0 ;

	for (i = 0 ; i < (vertices-1) ; i += 1) {
	    int		f_nochange = TRUE ;
	    for (j = 0 ; j < vertices ; j += 1) { /* edges(u,v) */
		const int	u = edges[j].src ;
	        if (resp[u].dist != INT_MAX) {
	            const int	d = resp[u].dist ;
	            const int	w = edges[j].weight ;
		    const int	v = edges[j].dst ;
	                if ((d+w) < resp[v].dist) {
	                    resp[v].dist = (d+w) ;
	                    resp[v].prev = u ;
			    f_nochange = FALSE ;
	                }
		} /* end if (reachable) */
	    } /* end for */
	    if (f_nochange) break ;
	} /* end for */

/* this is the famous "extra cycle" to check for negative paths */

	for (j = 0 ; j < vertices ; j += 1) {
		const int	u = edges[j].src ;
	        if (resp[u].dist != INT_MAX) {
	            const int	d = resp[u].dist ;
	            const int	w = edges[j].weight ;
		    const int	v = edges[j].dst ;
	                if ((d+w) < resp[v].dist) {
	                    rs = SR_LOOP ;
	                }
		} /* end if (reachable) */
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (bellmanford2) */


