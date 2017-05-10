
#include	<stdio.h>

int main()
{
	int	i ;

	double	ck = 2.4e9 ;
	double	l1 = 120.0 ;
	double	l2 = 170.0 ;
	double	p  ;


	for (i = 0 ; i < 6 ; i += 1) {

/* lower limit */

		p = 1.0 / l1 ;
		printf("year=%d perf1=%10.3f lat1=%10.3f\n",
			i,p,l1) ;

		p = p * 1.07 ;
		l1 = 1.0 / p ;

/* upper limit */

		p = 1.0 / l2 ;
		printf("year=%d perf2=%10.3f lat2=%10.3f\n",
			i,p,l2) ;

		p = p * 1.07 ;
		l2 = 1.0 / p ;

/* clock */
		printf("year=%d ck=%14.3f\n",i,ck) ;

		ck = ck * 1.4 ;

	}

	fclose(stdout) ;

	return 0 ;
}


