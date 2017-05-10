/* main (TESTUINT) */

/* see if you have an ANSI compiler or the K&R compiler */


/* revision history:

	= 1999-03-01, David A­D­ Morano

*/

#include	<sys/types.h>
#include	<limits.h>
#include	<stdio.h>


int main()
{
	unsigned int	ui = 0xff000000 ;

	if (ui > UCHAR_MAX) {
	    fprintf(stdout,"greater\n") ;
	} else {
	    fprintf(stdout,"less or equal\n") ;
	}

	return 0 ;
}
/* end subroutine (main) */


