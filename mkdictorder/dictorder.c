/* main (dictorder) */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Just print out 256 characters!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	isalnumlatin(int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int main()
{
	FILE		*fp = stdout ;
	int		n, i, c ;

	for (c = 0 ; c < 64 ; c += 1) {
	   if (isalnumlatin(c))
	       fprintf(fp,"%c\n",c) ;
	} /* end for */

	n = 32 ;
	for (c = 64 ; c < (128-n) ; c += 1) {
	   if (isalnumlatin(c))
	       fprintf(fp,"%c\n",c) ;
	   if (isalnumlatin(c+n))
	       fprintf(fp,"%c\n",(c+n)) ;
	} /* end for */

	n = 32 ;
	c = (128+64) ;
	for (i = 0 ; i < n ; (i += 1, c += 1)) {
	   if (isalnumlatin(c))
	       fprintf(fp,"%c\n",c) ;
	   if (isalnumlatin(c+n))
	       fprintf(fp,"%c\n",(c+n)) ;
	} /* end for */

	fflush(fp) ;

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


