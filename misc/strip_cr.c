/* program to strip comments from an assemble language program */

/*
	This program will remove all carriage return characters
	from the ends of all lines.
*/



#include	<stdio.h>

#include	"localmisc.h"


#define		EOL	'\n'		/* end of line mark */
#define		CR	'\r'


int main()
{
	int	c ;


/* come to start a new line */
top:
	c = getchar()  ;

	if (c == EOF) return OK ;

	if (c != CR) putchar(c) ;

	goto top ;

}
/* end of program */

