


#include	<stdio.h>


extern int	sub() ;


int bbsub()
{
	int	rs ;


	sub() ;

	rs = printf("bbsub: called\n") ;

	return rs ;
}



