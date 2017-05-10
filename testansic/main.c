/* main */

/* see if you have an ANSI compiler or the K&R compiler */


/* revision history:

	= 1999-03-01, David A­D­ Morano

*/


#include	<stdio.h>


int main()
{
	int		iw = -1 ;
	unsigned short	us = 1 ;
	const char	*fmt ;

	if (us < iw) {
	    fmt = "you have the correct K&R compiler\n" ;
	    fprintf(stdout,fmt) ;
	} else {
	    fmt = "you have the bad ANSI conforming compiler\n" ;
	    fprintf(stdout,fmt) ;
	}

	return 0 ;
}
/* end subroutine (main) */


