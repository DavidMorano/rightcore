/* mainbook */
/* lang=C++11 */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2010-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2010 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program indexes the words in a file and then allows queries on
	specified words.  Queies return the number of times the given word was
	found in the text file.  This program is a lttle bit more of a toy than
	anything else, but it is here if you want to use it.

	Synopsis:
	$ book <file> [<queryies>]


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<cstdlib>
#include	<cstring>
#include	<cinttypes>
#include	<exception>
#include	<new>
#include	<map>
#include	<unordered_map>
#include	<algorithm>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<estrings.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	VARDEBUGFNAME	"BOOK_DEBUGFILE"


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" int	sisub(cchar *,int,cchar *) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* global variables */


/* local structures (and methods) */

class wordcount {
	unordered_map<string,int>	db ;
public:
	wordcount() { } ;
	int load(cchar *) ;
	int query(cchar *) ;
} ;


/* forward references */

static int readline(ifstream &,char *,int) ;


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	wordcount	book ;
	int		rs ;
	cchar		*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	if ((argc > 1) && (argv[1] != NULL)) {
	    int		ai = 1 ;
	    cchar	*name = argv[ai++] ;
	    if ((rs = book.load(name)) >= 0) {
	        for ( ; argv[ai] != NULL ; ai += 1) {
		    cchar	*w = argv[ai] ;
		    rs = book.query(w) ;
		    if (rs < 0) break ;
		    cout << w << " " << rs << endl ;
		} /* end for */
	    }
	}

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


int wordcount::load(cchar *ifn)
{
	ifstream	is(ifn) ;
	int		rs = SR_OK ;
	int		c = 0 ;
	if (is.good()) {
	    string	s ;
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN+1] ;
	    while ((rs = readline(is,lbuf,llen)) > 0) {
		int	len = rs ;
		int	cl, sl ;
		cchar	*cp, *sp ;
		char	*tbuf ;
		if (lbuf[len-1] == '\n') len -= 1 ;
		lbuf[len] = '\0' ;
		sp = lbuf ;
		sl = len ;
		while ((cl = nextfield(sp,sl,&cp)) > 0) {
		    s.clear() ;
		    if ((tbuf = new(nothrow) char [len+1]) != NULL) {
			strwcpylc(tbuf,cp,cl) ;
		        s.append(tbuf,cl) ;
		        try {
		            int &val = db.at(s) ;
			    val += 1 ;
		        } catch (const exception& err) {
			    pair<string,int>	e(s,1) ;
			    db.insert(e) ;
		            c += 1 ;
		        }
		    } else {
			rs = SR_NOMEM ;
		    }
		    sl -= ((cp+cl)-sp) ;
		    sp = (cp+cl) ;
		} /* end while (processing line) */
	    } /* end while (reading lines) */
	} else {
	    cerr << "bad open on input" << endl ;
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (wordcount::load) */


int wordcount::query(cchar *wp)
{
	const int	len = strlen(wp) ;
	int		rs = SR_OK ;
	char		*tbuf ;
	if ((tbuf = new char [len+1]) != NULL) {
	    int	wl = strwcpylc(tbuf,wp,-1) - tbuf ;
	    try {
	        string	s(tbuf,wl) ;
		{
	            int	&val = db.at(s) ;
	            rs = val ;
		}
	    } catch (const exception& err) {
	        rs = 0 ;
	    }
	} else {
	    rs = SR_NOMEM ;
	}
	return rs ;
}
/* end subroutine (wordcount::query) */


int readline(ifstream &is,char *lbuf,int llen)
{
	int		rs = SR_OK ;
	if (is.getline(lbuf,llen)) {
	    rs = is.gcount() ;
	}
	return rs ;
}
/* end subroutine (readline) */


