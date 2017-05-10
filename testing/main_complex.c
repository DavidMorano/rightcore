


#include	<stdio.h>


int	funny = 0 ;


int main()
{
	complex double	a, b, c ;
	int		i ;
	int		es ;

	a = { 1.0, 2.0 } ;
	b = { 1.0, 2.0 } ;

	for (i = 0 ; i < 20 ; i += 1) {
		funny += i ;
		a += b + c + 1 ;
	}

	es = 0 ;
	if (a == 0)
		es = 1 ;

	return es ;
}


