/* gamma */

/* calculate the gamma function */


/* revision history:

	= 82/12/25, D. P. Mitchell

	This subroutine was originally written.


	= 91/10/26, D. P. Mitchell

	This was made a part of the 'chisquare' suite of
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


/******************************************************************************

	C program for floating point log gamma function

	gamma(x) computes the log of the absolute value of the gamma
	function.  The sign of the gamma function is returned in the
	external quantity signgam.

	The coefficients for expansion around zero are #5243 from Hart
	& Cheney; for expansion around infinity they are #5404.

	Calls log() and sin().


******************************************************************************/



#include <math.h>
#include <errno.h>



/* local defines */

#define M 6
#define N 8



/* external subroutines */


/* local structures */


/* forward references */

static double	pos(), neg(), asym() ;


/* global data */

int	signgam = 0;


/* local data */

static double goobie	= 0.9189385332046727417803297;
static double pi	= 3.1415926535897932384626434;

static double p1[] = {
	0.83333333333333101837e-1,
	-.277777777735865004e-2,
	0.793650576493454e-3,
	-.5951896861197e-3,
	0.83645878922e-3,
	-.1633436431e-2,
};

static double p2[] = {
	-.42353689509744089647e5,
	-.20886861789269887364e5,
	-.87627102978521489560e4,
	-.20085274013072791214e4,
	-.43933044406002567613e3,
	-.50108693752970953015e2,
	-.67449507245925289918e1,
	0.0,
};

static double q2[] = {
	-.42353689509744090010e5,
	-.29803853309256649932e4,
	0.99403074150827709015e4,
	-.15286072737795220248e4,
	-.49902852662143904834e3,
	0.18949823415702801641e3,
	-.23081551524580124562e2,
	0.10000000000000000000e1,
};



static double asym(arg)
double arg;
{
	double n, argsq;
	int i;


	argsq = 1./(arg*arg);
	for (n=0,i=M-1; i>=0; i--)
		n = n*argsq + p1[i];

	return ((arg-.5)*log(arg) - arg + goobie + n/arg);
}

static double neg(arg)
double arg;
{
	double temp;


	arg = -arg;
	temp = sin(pi*arg);

	if (temp == 0.) {
		errno = EDOM;
		return (HUGE);
	}

	if (temp < 0.) 
		temp = -temp;

	else 
		signgam = -1;

	return (-log(arg*pos(arg)*temp/pi));
}

static double pos(arg)
double arg;
{
	double n, d, s;
	register i;


	if (arg < 2.) 
		return (pos(arg+1.)/arg);

	if (arg > 3.) 
		return ((arg-1.)*pos(arg-1.));

	s = arg - 2.;
	for (n=0,d=0,i=N-1; i>=0; i--){
		n = n*s + p2[i];
		d = d*s + q2[i];
	}

	return (n/d);
}

double gamma(arg)
double arg;
{


	signgam = 1.;
	if (arg <= 0.) 
		return (neg(arg));

	if (arg > 8.) 
		return (asym(arg));

	return (log(pos(arg)));
}



