

#include	<stdio.h>



extern int	sub2() ;



int sub1()
{
	int	rs ;


	printf("sub1: calling sub2()\n") ;

	rs = sub2() ;

	return rs ;
}


