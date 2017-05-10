/* program to convert file into HEXADECIMAL */

/*
	David A.D. Morano
*/


#include	<stdio.h>

#include	"localmisc.h"



/*************************************************************************

	This program will read in an input file from standard input
	(file descriptor #0) and will format each byte of the file
	into HEX.  Each row of output will contain a maximun
	of 16 bytes formatted as HEX.


***************************************************************************/



int main()
{
	int	c, i ;


	i = 0 ;
	while (TRUE) {

	if ((i % 16) == 0) printf("%8.6X:",i) ;

	c = getchar() ;

	if (c == EOF) goto eof ;

	printf(" %02X",c) ;

	i++ ;
	if ((i % 16) == 0) putchar('\n') ;

	} /* end while */

eof:
	putchar('\n') ;

	return 0 ;
}



