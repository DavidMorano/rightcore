/* main */

/* remove white-space */



#include	<stdlib.h>
#include	<ctype.h>
#include	<stdio.h>


int main()
{
	int	rs ;
	int	i, c ;


	i = 0 ;
	while ((c = fgetc(stdin)) >= 0) {

	    if ((! isspace(c)) && (c != ':') && (c != '0')) {

	        fputc(c,stdout) ;

	        i += 1 ;
	        if (i >= 60) {

	            i = 0 ;
	            fputc('\n',stdout) ;
	        }
	    }
	}

	fputc('\n',stdout) ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



