/* change all leading tabs to 4 spaces */

#include	<stdio.h>


int main()
{
	int	c, leading ;

	leading = 1 ;
	while ((c = getchar()) != EOF) {

		if (leading && (c == '\t')) {

			putchar(' ') ;
			putchar(' ') ;
			putchar(' ') ;
			putchar(' ') ;

		} else if (c == '\n') {

			leading = 1 ;
			putchar('\n') ;

		} else {

			leading = 0 ;
			putchar(c) ;

		} ;

	} ; /* end while */

	fflush(stdout) ;

	return 0 ;
}

