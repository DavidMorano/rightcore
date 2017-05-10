
#include	<stdio.h>


main()
{
	long double	a, b, c ;


	printf("sizeof long double %d\n",
		sizeof(long double)) ;

	a = 2.3 ;
	b = 4.1 ;
	c = a + b ;
	printf("what is %Lf\n",c) ;

	fclose(stdout) ;

	return 0 ;
}


