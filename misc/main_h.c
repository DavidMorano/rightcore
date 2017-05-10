/* program to clear the terminal screen */

#include	<stdio.h>

#define		ESC	(char) 0x1B

int main()
{
	printf("%1c[H",ESC) ;

	fflush(stdout) ;

	return 0 ;
}
