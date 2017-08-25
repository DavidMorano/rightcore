/* main (mkdp) */


/* revision history:

	= 1998-06-29, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<stdio.h>


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	int		j = 0 ;
	int		b ;
	int		addr, size ;
	int		a, s ;
	int		dp ;

	fprintf(stdout,"static unsigned short dps[] = {\n") ;

	for (addr = 0 ; addr < 8 ; addr += 1) {

	    for (size = 0 ; size < 16 ; size += 1) {

	        dp = 0 ;
	        a = addr ;
	        for (s = 0 ; s < size ; s += 1) {

	            b = a + s ;
	            if (b >= 8) {
	                dp = 0xFFFF ;
	                break ;
	            } else {
	                dp |= (1 << b) ;
		    }

	        } /* end for */

	        if ((j % 8) == 0) {
	            fprintf(stdout,"\t") ;
	        } else {
	            fprintf(stdout," ") ;
		}

	        fprintf(stdout,"0x%04x,",dp) ;

	        if ((j % 8) == 7)
	            fprintf(stdout,"\n") ;

	        j += 1 ;

	    } /* end for */

	} /* end for */

	fprintf(stdout,"} ;\n") ;

	return 0 ;
}
/* end subroutine (main) */


