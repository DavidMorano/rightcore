/* main (mkiswhite) */


#define	CF_NEWLINE	0		/* include NL? */


/* revision history:

	= 1998-06-29, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This program makes a C-language fragment that consists of an array of
        characters. Elements of the array are set to zero or non-zero according
        to whether the attay is indexed by a character that is white-space or
        not.


******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<ascii.h>
#include	<localmisc.h>


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	int		i ;
	int		j = 0 ;
	int		rc ;
	char		*name = "iswhite" ;

	fprintf(stdout,"const unsigned char char_%s[] = %c\n",
	    name,CH_LBRACE) ;

	for (i = 0 ; i < 256 ; i += 1) {

	    rc = 0 ;
	    rc = rc || (i == ' ') ;
	    rc = rc || (i == '\t') ;
	    rc = rc || (i == '\v') ;
	    rc = rc || (i == '\f') ;
	    rc = rc || (i == '\r') ;

#if	CF_NEWLINE
	    rc = rc || (i == '\n') ;
#endif

	    if ((i & 7) == 0)
	        fprintf(stdout,"\t") ;

	    if ((i & 7) != 0)
	        fprintf(stdout," ") ;

	    fprintf(stdout,"0x%02x,",rc) ;

	    if ((i & 7) == 7)
	        fprintf(stdout,"\n") ;

	} /* end for */

	fprintf(stdout,"%c ;\n", CH_RBRACE) ;

	return 0 ;
}
/* end subroutine (main) */


