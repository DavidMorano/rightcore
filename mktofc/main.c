/* main (mktofc) */

/* Make To Folded Case */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-29, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program makes a C-language fragment that is an array of unsigned
        characters that constitutes a translation table used to fold characters
        down to their base case (or character).

        This is especially necessary for folding ISO-Ltin-1 characters down to
        the representative regular ASCII capital character.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	freadline(FILE *,char *,int) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	isalnumlatin(int) ;


/* forward references */

static int	printout(FILE *,const uchar *) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	FILE		*ifp = stdin ;
	FILE		*ofp = stdout ;
	int		i ;
	int		bch, ch ;
	int		len ;
	int		ll, cl ;
	uchar		a[256] ;
	const char	*lp ;
	const char	*cp ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS 
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

/* load up the default characters */

	for (i = 0 ; i < 256 ; i += 1) a[i] = i ;

/* load all other characters according to the standard-input file */

	while ((len = freadline(ifp,lbuf,LINEBUFLEN)) > 0) {

	    if (lbuf[len-1] == '\n') len -= 1 ;

	    if ((len == 0) || (lbuf[0] == '#')) continue ;

#if	CF_DEBUGS
	    debugprintf("main: line=>%t<\n",lbuf,len) ;
#endif

	    lp = lbuf ;
	    ll = len ;

	    i = 0 ;
	    while ((cl = nextfield(lp,ll,&cp)) > 0) {

		ch = (cp[0] & UCHAR_MAX) ;
		if (isalnumlatin(ch)) {
		    if (i++ > 0) {
		        a[ch] = bch ;
		    } else {
			bch = ch ;
		    }
		}

		ll -= ((cp + cl) - lp) ;
		lp = (cp + cl) ;

	    } /* end while */

	} /* end while (readling lines) */

/* print out the resulting array */

	printout(ofp,a) ;

/* done */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int printout(ofp,a)
FILE		*ofp ;
const uchar	a[] ;
{
	const int	n = 8 ;
	int		i, j ;
	int		order ;

	fprintf(ofp,"const unsigned char char_tofc[] = %c\n",CH_LBRACE) ;

	for (i = 0 ; i < (256/n) ; i += 1) {

	    fprintf(ofp,"\t") ;

	    for (j = 0 ; j < n ; j += 1) {

	        if (j > 0)
	            fprintf(ofp," ") ;

		order = ((i * n) + j) ;
	        fprintf(ofp,"0x%02x%s",
			a[order],((order != UCHAR_MAX) ? "," : "")) ;

	    } /* end for */

	    fprintf(ofp,"\n") ;

	} /* end for */

	fprintf(ofp,"%c ;\n", CH_RBRACE) ;

	fflush(ofp) ;

	return 0 ;
}
/* end subroutine (printout) */


