/* main (test complex) */
/* lang=C++ */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


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


