/* mainbook */
/* lang=C++11 */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program indexes the words in a file and then allows queries on
	specified words.  Queies return the number of times the given word was
	found in the text file.  This program is a lttle bit more of a toy than
	anything else, but it is here if you want to use it.

	Synopsis:

	$ book <file> [<queries>]


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<cstdlib>
#include	<cstring>
#include	<cinttypes>
#include	<exception>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<unordered_map>
#include	<map>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<baops.h>
#include	<estrings.h>
#include	<field.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */

#define	VARDEBUGFNAME	"BOOK_DEBUGFILE"


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" int	sfshrink(cchar *,int,cchar **) ;
extern "C" int	sisub(cchar *,int,cchar *) ;
extern "C" int	field_word(FIELD *,const uchar *,cchar **) ;
extern "C" int	isalphalatin(int) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* global variables */


/* local structures (and methods) */

static int	mkterms(uchar *) ;

class wordcount {
	unordered_map<string,int>	db ;
	uchar				wterms[256] ;
	int procline(string *,cchar *,int) ;
public:
	wordcount() { 
	    mkterms(wterms) ;
	} ;
	int load(cchar *) ;
	int query(cchar *) ;
} ;


/* forward references */

static int	mkterms(uchar *) ;
static int	readline(ifstream &,char *,int) ;
static int	procword(char *,cchar *,int) ;


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
	    } /* end if (book.load) */
	} /* end if (arguments) */

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
		int	cl ;
		cchar	*cp ;

		if (lbuf[len-1] == '\n') len -= 1 ;
		lbuf[len] = '\0' ;

		if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
		    rs = procline(&s,cp,cl) ;
		}

		if (rs < 0) break ;
	    } /* end while (reading lines) */
	} else {
	    cerr << "bad open on input" << endl ;
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (wordcount::load) */


int wordcount::procline(string *sp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;

	    while ((fl = field_word(&fsb,wterms,&fp)) >= 0) {
	        if (fl > 0) {
		    int		wl ;
		    cchar	*wp ;
	            if ((wl = sfword(fp,fl,&wp)) > 0) {
			char	*tbuf ;

		        sp->clear() ;
		        if ((tbuf = new(nothrow) char [wl+1]) != NULL) {
			    int	tl ;
			    if ((tl = procword(tbuf,wp,wl)) > 0) {
		                sp->append(tbuf,tl) ;
		                try {
		                    int &val = db.at(*sp) ;
			            val += 1 ;
		                } catch (const exception& err) {
			            pair<string,int>	e(*sp,1) ;
			            db.insert(e) ;
		                    c += 1 ;
		                }
			    } /* end if (procword) */
		        } else {
			    rs = SR_NOMEM ;
		        }

	            } /* end if (have word) */
	        } /* end if (positive) */
	        if (rs < 0) break ;
	    } /* end while (fielding words) */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (wordcount::procline) */


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


static int procword(char *tbuf,cchar *cp,int cl)
{
	return strwcpylc(tbuf,cp,cl) - tbuf ;
}
/* end subroutine (procword) */


static int mkterms(uchar *wterms)
{
	int		i ;

	for (i = 0 ; i < 32 ; i += 1) {
	    wterms[i] = 0xFF ;
	}

	BACLR(wterms,'_') ;
	BACLR(wterms,'-') ;

	BACLR(wterms,CH_SQUOTE) ;

	for (i = 0x20 ; i < 256 ; i += 1) {
	    if (isalphalatin(i)) {
	        BACLR(wterms,i) ;
	    }
	} /* end for */

	return SR_OK ;
}
/* end subroutine (mkterms) */


