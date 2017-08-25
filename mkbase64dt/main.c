/* main (mkbase64dt) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-29, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<localmisc.h>


/* external variables */

extern unsigned char	base64_et[] ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	int		i, j ;
	int		c = 0 ;

	for (i = 0 ; i < 256 ; i += 1) {

	    fprintf(stdout,((c == 0) ? "\t" : " ")) ;

	    for (j = 0 ; j < 64 ; j += 1) {
	        if (i == base64_et[j]) break ;
	    }

	    if (i == '=') {
	        fprintf(stdout,"0x%02X,",0xFE) ;

	    } else if (j >= 64) {
	        fprintf(stdout,"0x%02X,",0xFF) ;

	    } else {
	        fprintf(stdout,"0x%02X,",j) ;
	    }

	    c += 1 ;
	    if (c >= 8) {
	        c = 0 ;
	        fprintf(stdout,"\n") ;
	    }

	} /* end for (outer) */

	return 0 ;
}
/* end subroutine (main) */


