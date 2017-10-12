/* maingraph */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2017-08-25, David A­D­ Morano
	Written to examine Dijkstra's Algorithm.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We experiement with graph traversal.

	Notes:

	The Dijkstra Algorithm

	Synopsis:
	int dijkstra1(res1_t *resp,edges_t &edges,int vertices,int vstart)

	Arguments:
	resp		result array of structures
	edges		the edges in for of adjacency list (vector<list<edge_t>>
	vertixes	number of vertices
	vstart		starting vertez

	Returns:
	-		?


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<cstdlib>
#include	<cinttypes>
#include	<cstring>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<array>
#include	<vector>
#include	<list>
#include	<string>
#include	<set>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"dijkstra1.hh"
#include	"dijkstra2.hh"
#include	"bellmanford1.hh"
#include	"bellmanford2.hh"


/* name-spaces */

using namespace std ;


/* external subroutines */


/* global variables */


/* local structures (and methods) */

typedef struct graph_edge		edge1_t ;
typedef struct graph_res		res1_t ;

typedef vector<list<edge1_t>>		edges_t ;
typedef list<edge1_t>::iterator		edgeit_t ;


/* forward references */

static int edgesload(edges_t &) ;
static int edgesadd(edges_t &,int,int,int) ;
static int edgesvertices(edges_t &) ;
static int printgraph(edges_t &) ;
static int printlist(int,list<edge1_t> &) ;
static int printresult(res1_t *,int) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,const char **argv,const char **envv)
{
	const int	algos[] = { 1, 2, 3 } ;
	edges_t		es ;
	const int	nv = 6 ;


	edgesload(es) ;

	{
	    int	n = edgesvertices(es) ;
	    cout << "nv=" << n << endl ;
	}

	printgraph(es) ;

	{
	    res1_t	*rp ;
	    int		n = es.size() ;
	    if (n < nv) n = nv ;
	    if ((rp = new(nothrow) res1_t[n+1]) != NULL) {

	        for (auto al : algos) {
	            cout << "algo=" << al << " n=" << n << endl ;
	            switch (al) {
	            case 1:
	                dijkstra1(rp,es,n,0) ;
		        break ;
	            case 2:
	                dijkstra2(rp,es,n,0) ;
		        break ;
	            case 3:
	                bellmanford1(rp,es,n,0) ;
		        break ;
	            } /* end switch */
		    printresult(rp,n) ;
	        } /* end for */

		delete [] rp ;
	    } /* end if (m-a-f) */
	} /* end block */

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int edgesadd(edges_t &e,int u,int v,int w)
{
	edge1_t		tmp(v,w) ;
	const size_t	vsize = (uint) (u+1) ;
	if (e.size() < vsize) {
	    e.resize(vsize) ;
	}
	e[u].push_back(tmp) ;
	return 0 ;
}
/* end subroutine (edgesadd) */


static int edgesvertices(edges_t &e)
{
	int		vmax = 0 ;
	for (auto &v : e) {
	    edgeit_t	end = v.end() ;
	    edgeit_t	it = v.begin() ;
	    while (it != end) {
		if (it->dst > vmax) {
		    vmax = it->dst ;
		}
		it++ ;
	    } /* end while */
	}
	return (vmax+1) ;
}
/* end subroutine (edgesadd) */


static int printgraph(edges_t &es)
{
	const int	n = es.size() ;
	int		i ;
	for (i = 0 ; i < n ; i += 1) {
	    printlist(i,es[i]) ;
	}
	return i ;
}
/* end subroutine (printgraph) */


static int printlist(int v,list<edge1_t> &ev)
{
	cout << v ;
	for (auto v : ev) {
	   cout << " " << v.dst << ":" << v.weight ;
	}
	cout << endl ;
	return 0 ;
}
/* end subroutine (printgraph) */


static int printresult(res1_t *rp,int nr)
{
	int	i ;
	cout << "result\n" ;
	for (i = 0 ; i < nr ; i += 1) {
	    cout << setw(2) << i << " " << setw(2) << rp[i].dist ;
	    cout << " " << setw(2) << rp[i].prev << "\n" ;
	}
	return 0 ;
}
/* end subroutine (printresult) */


static int edgesload(edges_t &es)
{

	edgesadd(es,0,1,5) ;
	edgesadd(es,0,2,7) ;

	edgesadd(es,1,3,4) ;

	edgesadd(es,2,4,3) ;

	edgesadd(es,3,5,1) ;

	edgesadd(es,4,5,2) ;
	return 0 ;
}
/* end subroutine (edgesload) */


