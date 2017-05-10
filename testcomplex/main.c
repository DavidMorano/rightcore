/* main */


#include	<complex.h>
#include	<stdio.h>


int main()
{
	complex	double	a = 1 ;
	double		r, i ;

	a += 1.0*I ;

	r = creal(a) ;
	i = cimag(a) ;

	printf("a.re=%f\n",r) ;
	printf("a.im=%f\n",i) ;

	return 0 ;
}
/* end subroutine (main) */


