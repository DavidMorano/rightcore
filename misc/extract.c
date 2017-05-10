
#include	"stdio.h"


int main()
{
	int	c ;

/* fast forward to first non-white space character */

skip:
		c = getchar() ;
		if (c == EOF) goto exit ;

		if (c == ' ' || c == '\t' || c == '\n') goto skip ;

		putchar(c) ;

copy:
		c = getchar() ;
		if (c == EOF) goto exit ;

		if (c != ' ' && c != '\t' && c != '\n') {

			putchar(c) ;

			goto copy ;

		} else {

			putchar('\n') ;

			goto skip ;
		} ;

exit:
	return(0) ;
}



