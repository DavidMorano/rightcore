/* program to strip lines starting w/ ".mc" */


#include	<stdio.h>


main()
{
	int	c, c0, c1, c2 ;


top:
	c0 = getchar() ;
	if (c0 == EOF) return (0L) ;

	if (c0 != '.') { putchar(c0) ; goto finish ; }

	c1 = getchar() ;
	if (c1 == EOF) return (0L) ;

	if (c1 != 'm') { putchar(c0) ; putchar(c1) ; goto finish ; }

	c2 = getchar() ;
	if (c2 == EOF) return (0L) ;

	if (c2 != 'c') 
		{ putchar(c0) ; putchar(c1) ; putchar(c2) ; goto finish ; }



	while ((c = getchar()) != '\n') ;

	if (c == EOF) return 0 ;

	goto	top ;

finish:
	c = getchar() ;
	if (c == EOF) return 0 ;

	while (c != '\n') {

		putchar(c) ;

		c = getchar() ;
		if (c == EOF) return 0 ;
	} ;

	putchar('\n') ;

	goto	top ;
}
/* end of program */

