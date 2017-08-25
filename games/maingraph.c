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
	int dijkstra1(res1_t *resp,edges1_t &edges,int vertices,int vstart)

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
#include	<algorithm>
#include	<array>
#include	<vector>
#include	<list>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"dijkstra1.hh"
#include	"dijkstra2.hh"


/* name-spaces */

using namespace std ;


/* external subroutines */


/* global variables */


/* local structures (and methods) */

typedef struct dijkstra1_edge		edge1_t ;
typedef struct dijkstra1_res		res1_t ;

typedef struct dijkstra2_edge		edge2_t ;
typedef struct dijkstra2_res		res2_t ;

typedef vector<list<edge1_t>>		edges1_t ;
typedef list<edge1_t>::iterator		edgeit1_t ;


/* forward references */

static int edgesadd(edges1_t &,int,int,int) ;
static int edgesvertices(edges1_t &) ;
static int printgraph(edges1_t &) ;
static int printlist(int,list<edge1_t> &) ;
static int printresult(res1_t *,int) ;


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	edges1_t	es ;
	const int	nv = 6 ;

	edgesadd(es,0,1,5) ;
	edgesadd(es,0,2,7) ;

	edgesadd(es,1,3,4) ;

	edgesadd(es,2,4,3) ;

	edgesadd(es,3,5,1) ;

	edgesadd(es,4,5,2) ;

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

	        dijkstra1(rp,es,n,0) ;

		printresult(rp,n) ;

		delete [] rp ;
	    } /* end if (m-a-f) */
	} /* end block */

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int edgesadd(edges1_t &e,int u,int v,int w)
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


static int edgesvertices(edges1_t &e)
{
	int		vmax = 0 ;
	for (auto &v : e) {
	    edgeit1_t	end = v.end() ;
	    edgeit1_t	it = v.begin() ;
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


static int printgraph(edges1_t &es)
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


