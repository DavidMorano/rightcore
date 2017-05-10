/* program to remove leading white space */


#include	<stdio.h>


int main()
{
	int	c, leading ;

	leading = 1 ;
	while ((c = getchar()) != EOF) {

		if (leading && ((c == ' ') || (c == '\t'))) {

			continue ;

		} else if (c == '\n') {

			leading = 1 ;
			putchar('\n') ;

		} else {

			leading = 0 ;
			putchar(c) ;

		} ;

	} ; /* end while */

	fflush(stdout) ;
}

