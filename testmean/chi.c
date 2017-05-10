/* chi */

/* *	Chi-Square Cumulative Distribution */


/* revision history:

	= 91/10/26, D. P. Mitchell

	This was written as part of the 'chisquare' suite of
 	randomness tests.


	= 96/03/27, David A­D­ Morano

	I adapted this subroutine for use in my own randomness
	testing.


	= 01/06/23, David A­D­ Morano

	I had to add an external reference declaration for the 'sqrt()'
	math function since stupid Sun Solaris 8 messed up for forgot
	to put it in the standard 'math.h' header include file !
	All hail Sun Microsystems !


*/


#include	<math.h>



/* local defines */

#define TWOPI 6.2831853071795864769252864



/* external subroutines */

extern double	gamma() ;

#if	defined(SOLARIS) && (SOLARIS == 8)
extern double	sqrt(double) ;
#endif





double nCHI(x, n)
double x;
int n;
{
	double nu;
	double y;


	nu = (double)n;
	y = (pow(x/nu, 1.0/3.0) - (1.0 - 2.0/(9.0*nu)))
		/ sqrt(2.0/(9.0*nu));

	y = (1.0 + erf(y/sqrt(2.0))) * 0.5;

	return y;
}

double CHI(x, n)
double x;
int n;
{
	double nu;
	double y;


	nu = (double)n;
	if (n > 50)
		return nCHI(x, n);

	if (x > 100.0)
		return 1.0;

	if (n == 1) {
		y = erf(sqrt(0.5*x));

		return y;
	}
	if (n == 2) {
		y = 1.0 - exp(-0.5*x);

		return y;
	}

	y = CHI(x, n - 2)
	 - exp(0.5*(nu - 2.0)*log(0.5*x) - 0.5*x - gamma(0.5*nu));

	return y;
}

double chi(x, n)
double x;
int n;
{


	return CHI(x, n);
}



