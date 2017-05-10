

#include	<stdio.h>




extern int	sub() ;



int caller()
{
	int	rs ;


	printf("subcaller: calling sub()\n") ;

	rs = sub() ;

	return rs ;
}


