/* mainequ1 */
#include	<stdlib.h>
#include	<math.h>
#include	<stdio.h>
static int	equ(double,double) ;

int main(int argc,const char **argv,const char **envv)
{
	double		n = 100 ;
	double		x, y ;
	int		f = 0 ;

	for (x = -n ; x < n ; x += 0.02) {
	    for (y = -n ; y < n ; y += 0.02) {
		if (equ(20,((x*x) + x*y))) {
		printf("outer x=%f y=%f\n",x,y) ;
		    if (equ(30,(y*y + x*y))) {
		printf("inner x=%f y=%f\n",x,y) ;
			f = 1 ;
			break ;
		    }
		}
	    }
	} /* end for */

	if (f) {
	    printf("x=%u y=%u\n",x,y) ;
	}

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int equ(double a,double b)
{
	double		small = 0.1 ;
	return (fabs(a-b) < small) ;
}
/* end subroutine (equ) */


