/* main (web traffic) */


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

#define	CF_WEB2		1
#define	CF_WEB3		0
#define	CF_WEB4		1
#define	CF_WEB5		1
#define	CF_WEB6		1


/* name spaces */

using namespace	std ;


/* forward references */

static int readline(ifstream &,char *,int) ;

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
	const int	a[] = { 8, 6, 9, 4, 8, 8, 3, 10, 2, 1 } ;
	const int	n = nelem(a) ;
	int		ans ;

	{
	    int	out[n+1] ;
	    printa(a,n) ;
	    web1(out,a,n) ;
	    printa(out,n) ;
	}

#if	CF_WEB2
	{
	    int	out[n+1] ;
	    printa(a,n) ;
	    web2(out,a,n) ;
	    printa(out,n) ;
	}
#endif /* CF_WEB2 */

#if	CF_WEB3
	{
	    int	out[n+1] ;
	    printa(a,n) ;
	    web3(out,a,n) ;
	    printa(out,n) ;
	}
#endif /* CF_WEB3 */

#if	CF_WEB4
	{
	    int	out[n+1] ;
	    printa(a,n) ;
	    web4(out,a,n) ;
	    printa(out,n) ;
	}
#endif /* CF_WEB4 */

#if	CF_WEB5
	{
	    int	out[n+1] ;
	    printa(a,n) ;
	    web5(out,a,n) ;
	    printa(out,n) ;
	}
#endif /* CF_WEB5 */

#if	CF_WEB6
	{
	    int	out[n+1] ;
	    printa(a,n) ;
	    web6(out,a,n) ;
	    printa(out,n) ;
	}
#endif /* CF_WEB6 */

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
		    j += (out[j] - 1) ;
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
#ifdef	COMMENT
	        ans = web4_proc(out+1,data+1,n-1) ;
#else
	        ans = findincer(out+1,data+1,n-1,v) ;
#endif
	    }
	}
	out[0] = ans ;
	return ans ;
}
/* end subroutine (web4_proc) */


static void web5(int *out,const int *data,int n)
{
	if (n > 0) {
	    int 	i ;
	    out[n-1] = -1 ;
	    for (i = (n-1) ; i >= 0 ; i -= 1) {
	        const int	v = data[i] ;
	        out[i] = findincer(out+i+1,data+i+1,n-i-1,v) ;
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
		out[0] = ans ;
	    }
	}
	return ans ;
}
/* end subroutine (web6_recurse) */


static int readline(ifstream &is,char *lbuf,int llen)
{
	int		rs = SR_OK ;
	if (is.getline(lbuf,llen)) {
	    rs = is.gcount() ;
	}
	return rs ;
}
/* end subroutine (readline) */


static void printa(const int *a,int n)
{
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    cout << " " << setw(2) << a[i] ;
	}
	cout << endl ;
}
/* end subroutine (printa) */


