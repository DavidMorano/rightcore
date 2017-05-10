/* main (mkdictorder) */

/* make a translation table to provide dictionary-order for characters */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program makes an array 256 short integers.  

        This array serves as a sort of translation table for use in finding the
        dictionary-collating-ordinal number of a latin character. The array is
        indexed by the latin character (an 8-bit clean character) and returns
        the short integer representing its dictionary-collating-ordinal number.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	freadline(FILE *,char *,int) ;
extern int	isalnumlatin(int) ;


/* forward references */

static int	printout(FILE *,const short *) ;


/* local variables */


/* exported subroutines */


int main()
{
	FILE		*ifp = stdin ;
	FILE		*ofp = stdout ;
	const int	llen = LINEBUFLEN ;
	int		i ;
	int		order ;
	int		ch ;
	int		size ;
	int		len ;
	short		a[256] ;
	char		lbuf[LINEBUFLEN + 1] ;

	size = 256 * sizeof(short) ;
	memset(a,0,size) ;

/* load some special characters (special handling) */

	order = 100 ;
	{
	    cchar	*sp = "­-'`´  " ;
	    for (i = 0 ; sp[i] != '\0' ; i += 1) {
		ch = MKCHAR(sp[i]) ;
		a[ch] = order++ ;
	    }
	}

/* load all other characters according to the standard-input file */

	order = 1000 ;
	while ((len = freadline(ifp,lbuf,llen)) > 0) {

	    if (lbuf[len-1] == '\n') len -= 1 ;

	    if ((len == 0) || (lbuf[0] == '#')) continue ;

	    for (i = 0 ; i < len ; i += 1) {
		ch = MKCHAR(lbuf[i]) ;
		if (isalnumlatin(ch)) {
		    a[ch] = order++ ;
		}
	    } /* end for */

	} /* end while (readling lines) */

/* print out the results */

	printout(ofp,a) ;

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int printout(ofp,a)
FILE		*ofp ;
const short	a[] ;
{
	int		i, j ;
	int		order ;

	fprintf(ofp,"const short	char_dictorder[] = %c\n",CH_LBRACE) ;

	for (i = 0 ; i < 32 ; i += 1) {

	    fprintf(ofp,"\t") ;

	    for (j = 0 ; j < 8 ; j += 1) {

	        if (j > 0)
	            fprintf(ofp," ") ;

		order = ((i * 8) + j) ;
	        fprintf(ofp,"0x%04x%s",
			a[order],((order != UCHAR_MAX) ? "," : "")) ;

	    } /* end for */

	    fprintf(ofp,"\n") ;

	} /* end for */

	fprintf(ofp,"%c ;\n", CH_RBRACE) ;

	return 0 ;
}
/* end subroutine (printout) */


