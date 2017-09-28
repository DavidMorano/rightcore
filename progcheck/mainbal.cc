/* main (bal) */


/* revision history:

	= 2017-08-25, David A­D­ Morano
	Written originally as a response to a question.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	I: Garret (2017-08-25)


*******************************************************************************/

#include	<envstandards.h>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<set>
#include	<vector>
#include	<list>
#include	<stack>
#include	<deque>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* name spaces */

using namespace	std ;


/* local structures */

typedef std::set<std::string>	res_t ;

struct StrRecurse {
	int		N = 0 ;
	StrRecurse() { } ;
	StrRecurse(int aN) : N(aN) { } ;
	StrRecurse &operator = (int aN) {
	    N = aN ;
	    return (*this) ;
	} ;
	bool mkstr(set<string> &,string &,int,int,int) ;
} ;


bool StrRecurse::mkstr(res_t &res,string &s,int c,int i,int w)
{
	bool	f = false ;
	if (i == N) {
	    res.insert(s) ;
	    f = true ;
	} else {
	    if (w) {
		if (c < (N-i)) {
		    s[i] = '(' ;
		    c += 1 ;
	            if (mkstr(res,s,c,i+1,0) == false) {
	                mkstr(res,s,c,i+1,1) ;
		    }
		}
	    } else {
		if (c > 0) {
		    s[i] = ')' ;
		    c -= 1 ;
	    	    if (mkstr(res,s,c,i+1,0) == false) {
	    	        mkstr(res,s,c,i+1,1) ;
		    }
		}
	    }
	}
	return f ;
}
/* end method (StrRecurse::mkstr) */


/* forward references */

static void	printres(const res_t &res) ;

static res_t	bal1(int) ;
static res_t	bal2(int) ;
static res_t	bal3(int) ;


/* exported subroutines */


bool IsBalanced1(const std::string &input) {
  bool		f = true ;
  int		count = 0 ;
  for (auto ch:input) {
    if (ch == '(') { 
      count += 1 ;
    } else if (ch == ')') {
      if (count == 0) {
        f = false ;
        break ;
      }
      count -= 1 ;
    }
  } /* end for */
  if (f && (count > 0)) f = false ; 
  return f ;
} 
/* end subroutine (IsBalanced1) */


bool IsBalanced2(const std::string &input) {
  bool		f = true ;
  const int	n = input.length() ;
  int		count = 0 ;
  for (int i = 0 ; i < n ; i += 1) {
    const int	ch = input[i] ;
    if (ch == '(') { 
      count += 1 ;
      if (count > (n-i)) {
	f = false ;
	break ;
      }
    } else if (ch == ')') {
      if (count == 0) {
        f = false ;
        break ;
      }
      count -= 1 ;
    }
  } /* end for */
  return f ;
} 
/* end subroutine (IsBalanced2) */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	const int	algos[] = { 1, 2, 3 } ;
	const int	lengths[] = { 0, 1, 2, 4, 6, 8 } ;

	for (auto n : lengths) {
	    for (auto al : algos) {
	        set<string>	res ;
	        cout << "algo=" << al << " n=" << n << endl ;
	        switch (al) {
	        case 1:
	            res = bal1(n) ;
		    break ;
	        case 2:
	            res = bal2(n) ;
		    break ;
	        case 3:
	            res = bal3(n) ;
		    break ;
	        } /* end switch */
	        printres(res) ;
	    } /* end for */
	} /* end for */

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static res_t bal1(const int N) {
  res_t		res ;
  StrRecurse	mk(N) ;
  string	s ;
  if (N == 0) {
    s = "" ;
    res.insert(s) ;
  } else if ((N & 1) == 0) {
    s.resize(N) ;
    mk.mkstr(res,s,0,0,1) ;
  }
  return res ;
}
/* end subroutine (bal1) */


struct bal2_item {
	int	c = 0 ; /* count */
	int	i = 0 ; /* index */
	int	w = 0 ; /* which-type */
	bal2_item() { } ;
	bal2_item(int _c,int _i,int _w) : c(_c), i(_i), w(_w) { } ;
	bal2_item(initializer_list<int> il) {
	    int	p = 0 ;
	    for (auto &e : il) {
		switch (p++) {
		case 0:
		    c = e ;
		    break ;
		case 1:
		    i = e ;
		    break ;
		case 2:
		    w = e ;
		    break ;
		} /* end switch */
	    } /* end for */
	} ;
	bal2_item &operator = (const bal2_item &wi) = default ;
	bal2_item &operator = (initializer_list<int> il) {
	    int	p = 0 ;
	    c = 0 ;
	    i = 0 ;
	    w = 0 ;
	    for (auto &e : il) {
		switch (p++) {
		case 0:
		    c = e ;
		    break ;
		case 1:
		    i = e ;
		    break ;
		case 2:
		    w = e ;
		    break ;
		} /* end switch */
	    } /* end for */
	    return (*this) ;
	} ;
	/* do not need 'move' constructor or assignment (too simple) */
} ;

static void bal2_push(stack<bal2_item> &s,int c,int i,int w)
{
   bal2_item	wi(c,i,w) ;
   s.push(wi) ;
}

static res_t bal2(const int N) {
  res_t		res ;
  string	s ;
  if (N == 0) {
    s = "" ;
    res.insert(s) ;
  } else if ((N & 1) == 0) {
    stack<bal2_item>	work ;
    bal2_item		wi ;
    int			c = 1 ;
    int			i, w ;
    s.resize(N) ;
    s[0] = '(' ;
    bal2_push(work,c,1,0) ;
    bal2_push(work,c,1,1) ;
    while (! work.empty()) {
      wi = work.top() ;
      c = wi.c ;
      i = wi.i ;
      w = wi.w ;
      work.pop() ;
	if (i == N) {
	    res.insert(s) ;
	} else {
	    if (w) {
		if (c < (N-i)) {
		    s[i] = '(' ;
		    c += 1 ;
	            bal2_push(work,c,i+1,0) ;
		    if ((i+1) < N) {
	                bal2_push(work,c,i+1,1) ;
		    }
		}
	    } else {
		if (c > 0) {
		    s[i] = ')' ;
		    c -= 1 ;
	            bal2_push(work,c,i+1,0) ;
		    if ((i+1) < N) {
	                bal2_push(work,c,i+1,1) ;
		    }
		}
	    }
	} /* end if */
    } /* end while */
  } /* end if */
  return res ;
}
/* end subroutine (bal2) */


struct bal3_item {
	int	c = 0 ; /* count */
	int	i = 0 ; /* index */
	int	w = 0 ; /* which-type */
	bal3_item() { 
	} ;
	bal3_item(int _c,int _i,int _w) : c(_c), i(_i), w(_w) { 
	} ;
	bal3_item &operator = (const bal3_item &wi) = default ;
	/* do not need 'move' constructor or assignment (too simple) */
} ;

static void bal3_push(stack<bal3_item> &work,int c,int i)
{
	const int	n = 2 ;
	for (int w = 0 ; w < n ; w += 1) {
	    bal3_item	wi(c,i,w) ;
	    work.push(wi) ;
	}
}
/* end subroutine (bal3_push) */

static res_t bal3(const int N) {
	res_t		res ;
	string		s ;
	if (N == 0) {
	    s = "" ;
	    res.insert(s) ;
	} else if ((N & 1) == 0) {
	    stack<bal3_item>	work ;
	    bal3_item		wi ;
	    int			c = 1 ;
	    int			i, w ;
	    s.resize(N) ;
	    s[0] = '(' ;
	    bal3_push(work,c,1) ;
	    while (! work.empty()) {
	        wi = work.top() ;
	        c = wi.c ;
	        i = wi.i ;
	        w = wi.w ;
	        work.pop() ;
	        if (i < N) {
	            switch (w) {
		    case 1:
	                if (c < (N-i)) {
	                    s[i] = '(' ;
	                    c += 1 ;
	                    if ((i+1) == N) {
	                        res.insert(s) ;
	                    } else {
	                        bal3_push(work,c,i+1) ;
	                    }
	                }
			break ;
		    case 0:
	                if (c > 0) {
	                    s[i] = ')' ;
	                    c -= 1 ;
	                    if ((i+1) == N) {
	                        res.insert(s) ;
	                    } else {
	                        bal3_push(work,c,i+1) ;
	                    }
	                }
			break ;
	            } /* end switch */
	        } /* end if */
	    } /* end while */
	} /* end if */
	return res ;
}
/* end subroutine (bal3) */


static void printres(const res_t &res)
{
	for (auto &s : res) {
	    cout << s << endl ;
	}
}
/* end subroutine (printres) */


