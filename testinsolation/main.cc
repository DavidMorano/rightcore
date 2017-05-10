/* main */


#include	<sys/types.h>
#include	<sys/param.h>

#include	<iostream>
#include	<math.h>

#include	<vsystem.h>
#include	<exitcodes.h>

#include	"localmisc.h"



/* forward refernces */

static double	sum_sin(double,double) ;





int main(int argc,char **argv)
{
	double	y, x ;
	double	pi = 4.0 * atan(1) ;
	double	inc = 0.01 ;
	double	lower, upper ;
	double	sum ;
	double	avg, avg1, avg2 ;
	double	period ;
	double	factor ;

	int	rs = SR_OK ;


	std::cout << "pi=" << pi << "\n" ;

	sum = sum_sin(0.0,pi) ;
	period = (pi - 0.0) ;
	avg = sum / period ;
	avg1 = avg ;
	std::cout << "sum=" << sum << "\n" ;
	std::cout << "avg=" << avg << "\n" ;

	lower = pi / 3.0 ;
	upper = lower * 2.0 ;
	sum = sum_sin(lower,upper) ;
	period = (upper - lower) ;
	avg = sum / period ;
	avg2 = avg ;
	std::cout << "sum=" << sum << "\n" ;
	std::cout << "avg=" << avg << "\n" ;

	factor = avg2 / avg1 ;
	std::cout << "fac=" << factor << "\n" ;

	return EX_OK ;
}
/* end subroutine (main) */


/* local subroutines */


static double sum_sin(double lower,double upper)
{
	double	x, y ;
	double	inc = 0.01 ;
	double	sum = 0.0 ;


	for (x = lower ; x < upper ; x += inc) {
		y = sin(x) ;
		sum += (y * inc) ;
	} /* end for */

	return sum ;
}
/* end subroutine (sum_sin) */



