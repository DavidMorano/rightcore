/* main */

/* log check */


#define	F_DEBUGS	1


/* revision history :

	= 02/01/28, David A­D­ Morano

	I started this as a fast way to calculte this thing.


*/




#include	<sys/types.h>
#include	<stdlib.h>
#include	<math.h>
#include	<stdio.h>

#include	"misc.h"



/* external subroutines */







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	double	bpa, bma ;
	double	lbpa, lbma ;
	double	nf ;
	double	cumprob ;

	int	rs, i ;
	int	fd_deb ;

	char	*progname ;
	char	*cp ;


	{
		double	f1, f2, x, y, b ;


		b = 10.0 ;
		for (x = 1.0 ; x < 10.0 ; x += 1.0) {

			f1 = log(x) / log(b) ;

			f2 = log10(x) ;

			fprintf(stdout,"f1=%17.4f f2=%17.4f\n",
				f1,f2) ;

		}


	}



ret2:
	fclose(stdout) ;

ret1:
	fclose(stderr) ;

	return 0 ;

badargval:
	fprintf(stderr,"%s: invalid branch prediction accuracy given\n",
		progname) ;

	fclose(stderr) ;

	return 1 ;
}
/* end subroutine (main) */




