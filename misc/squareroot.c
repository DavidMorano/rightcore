/* squareroot */

#include	<math.h>
#include	<stdio.h>

int main()
{
	double		x = 0.0 ;


	for (x = 0.0 ; x < 100.0 ; x += 0.01) {
            double a ;
	    a = sqrt(x+15.0) + sqrt(x) ;
 	    if (a > 15.0) break ;
	}

	printf("x=%f\n",x) ;
	return 0 ;
}
/* end subroutine (main) */


