
#include	<stdio.h>
#include	<complex>

using namespace	std ;

int main()
{
	complex<double>	a(1.0,2.0) ;
	double		ar, ai ;
	ar = a.real() ;
	ai = a.imag() ;
	printf("a.real=%f a.ima=%f\n",ar,ai) ;
	return 0 ;
}
/* end subroutine (main) */


