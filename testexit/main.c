/* main */

#include	<stdio.h>

int main(argc,const char **argv,const char **envv)
{
	int		a ;
	int		b ;

	b = 1 ;
	a = b ;

	if (b > 1) exit() ;

	a = 2 ;
	printf("a="%d\n",a) ;

	return 0 ;
}
/* end subroutine (main) */


