/* main (web traffic) */


/* revision history:

	= 2017-08-10, David A­D­ Morano
	Written originally as a response to an interview question.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<algorithm>
#include	<functional>
#include	<vector>
#include	<list>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* name spaces */

using namespace	std ;


/* forward references */

static void printa(const int *,int) ;
static void web1(int *,const int *,int) ;
static void web2(int *,const int *,int) ;
static void web3(int *,const int *,int) ;
static void web4(int *,const int *,int) ;
static void web5(int *,const int *,int) ;
static void web6(int *,const int *,int) ;

static int findinc(int *,const int *,int,int) ;
static int findincer(int *,const int *,int,int) ;

static int web3_recurse(int *,const int *,int) ;
static int web4_recurse(int *,const int *,int) ;
static int web4_proc(int *,const int *,int) ;

static int web6_recurse(int *,const int *,int) ;


int main(int argc,cchar **argv,cchar **envv)
{
	const int	algos[] = { 1, 2, 3, 4, 5, 6 } ;
	const int	a[] = { 8, 6, 9, 4, 8, 8, 3, 10, 2, 1 } ;
	const int	n = nelem(a) ;
	int		ans ;

	for (auto al : algos) {
	    int	out[n+1] ;
	    cout << "algo=" << al << endl ;
	    printa(a,n) ;
	    switch (al) {
	    case 1:
	        web1(out,a,n) ;
		break ;
	    case 2:
	        web2(out,a,n) ;
		break ;
	    case 3:
	        web3(out,a,n) ;
		break ;
	    case 4:
	        web4(out,a,n) ;
		break ;
	    case 5:
	        web5(out,a,n) ;
		break ;
	    case 6:
	        web6(out,a,n) ;
		break ;
	    } /* end switch */
	    printa(out,n) ;
	} /* end for */

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static void web1(int *out,const int *data,int n)
{
	if (n > 0) {
	    int	i ;
	    out[n-1] = -1 ;
	    for (i = 0 ; i < (n-1) ; i += 1) {
	        int	j ;
	        out[i] = -1 ;
	        for (j = (i+1) ; j < n ; j += 1) {
		    const int	v = data[i] ;
	            if (v < data[j]) {
	                out[i] = (j-i) ;
	                break ;
	            }
	        }
	    } /* end for */
	}
}
/* end subroutine (web1) */


static void web2(int *out,const int *data,int n)
{
	if (n > 0) {
	    int 	i ;
	    out[n-1] = -1 ;
	    for (i = (n-1) ; i >= 0 ; i -= 1) {
	        const int	v = data[i] ;
	        out[i] = findinc(out+i+1,data+i+1,n-i-1,v) ;
	    }
	}
}
/* end subroutine (web2) */


static int findinc(int *out,const int *data,int n,int v)
{
	int		ans = -1 ;
	if (n > 0) {
	    if (v < data[0]) {
	        ans = 1 ;
	    } else if (v == data[0]) {
	        if (out[0] > 0) {
	            ans = (out[0] + 1) ;
	        }
	    } else { /* we are greater than following */
	        if (out[0] > 0) {
	            ans = findincer(out,data,n,v) ;
	        }
	    }
	}
	return ans ;
}
/* end subroutine (findinc) */


static int findincer(int *out,const int *data,int n,int v)
{
	int		ans = -1 ;
	int		j ;
	for (j = 0 ; j < n ; j += 1) {
	    if (v < data[j]) {
	        ans = (j+1) ;
	        break ;
	    } else if (v == data[j]) {
	        if (out[j] > 0) {
	            ans = (out[j]+j+1) ;
	        }
	        break ;
	    } else if (v > data[j]) {
	        if (out[j] > 0) {
		    j += (out[j] - 1) ; /* skip ahead */
		} else {
		    break ;
		}
	    }
	} /* end for */
	return ans ;
}
/* end subroutine (findincer) */


static void web3(int *out,const int *data,int n)
{
	if (n > 0) {
	    web3_recurse(out,data,n) ;
	}
}
/* end subroutine (web3) */


static int web3_recurse(int *out,const int *data,int n)
{
	int		ans = -1 ;
	if (n > 0) {
	    out[n-1] = -1 ;
	    if (n > 1) {
	        web3_recurse(out+1,data+1,n-1) ;
	        {
	            const int	v = data[0] ;
	            if (v < data[1]) {
	                ans = 1 ;
	            } else if (v == data[1]) {
	                if (out[1] > 0) {
	                    ans = (out[1] + 1) ;
	                }
	            } else { /* we are greater than following */
	                if (out[1] > 0) {
	                    ans = findincer(out+1,data+1,n-1,v) ;
	                }
	            }
	        }
	        out[0] = ans ;
	    }
	}
	return ans ;
}
/* end subroutine (web3_recurse) */


static void web4(int *out,const int *data,int n)
{
	if (n > 0) {
	    web4_recurse(out,data,n) ;
	}
}
/* end subroutine (web4) */


static int web4_recurse(int *out,const int *data,int n)
{
	int		ans = -1 ;
	if (n > 0) {
	    out[n-1] = -1 ;
	    if (n > 1) {
	        web4_recurse(out+1,data+1,n-1) ;
	        web4_proc(out,data,n) ;
	    }
	}
	return ans ;
}
/* end subroutine (web4_recurse) */


static int web4_proc(int *out,const int *data,int n)
{
	const int	v = data[0] ;
	int		ans = -1 ;
	if (v < data[1]) {
	    ans = 1 ;
	} else if (v == data[1]) {
	    if (out[1] > 0) {
	        ans = (out[1] + 1) ;
	    }
	} else { /* we are greater than following */
	    if (out[1] > 0) {
	        ans = findincer(out+1,data+1,n-1,v) ;
	    }
	}
	out[0] = ans ;
	return ans ;
}
/* end subroutine (web4_proc) */


static void web5(int *out,const int *data,int n)
{
	if (n > 0) {
	    out[n-1] = -1 ;
	    if (n > 1) {
	        int	max = data[n-1] ;
	        int 	i ;
	        for (i = (n-2) ; i >= 0 ; i -= 1) {
	            const int	v = data[i] ;
		    out[i] = -1 ;
		    if (v < max) {
	                out[i] = findincer(out+i+1,data+i+1,n-i-1,v) ;
		    }
		    if (data[i] > max) max = data[i] ;
	        }
	    }
	}
}
/* end subroutine (web5) */


static void web6(int *out,const int *data,int n)
{
	if (n > 0) {
	    web6_recurse(out,data,n) ;
	}
}
/* end subroutine (web6) */


static int web6_recurse(int *out,const int *data,int n)
{
	int		ans = -1 ;
	if (n > 0) {
	    out[n-1] = -1 ;
	    if (n > 1) {
		const int	v = data[0] ;
	        web6_recurse(out+1,data+1,n-1) ;
	        ans = findincer(out+1,data+1,n-1,v) ;
	    }
	    out[0] = ans ;
	}
	return ans ;
}
/* end subroutine (web6_recurse) */


static void printa(const int *a,int n)
{
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    cout << " " << setw(2) << a[i] ;
	}
	cout << endl ;
}
/* end subroutine (printa) */


