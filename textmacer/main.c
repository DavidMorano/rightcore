/* main */




#include	<stdio.h>




int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	FILE	*ifp = stdin ;
	FILE	*ofp = stdout ;

	int	c ;


	while ((c = fgetc(ifp)) >= 0) {

		if (c == '\r')
			c = '\n' ;

		fputc(c,ofp) ;

	} /* end while */

	return 0 ;
}
/* end subroutine (main) */



