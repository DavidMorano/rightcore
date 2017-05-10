/* main */

#define	F_CEIL		0




#include	<math.h>
#include	<stdio.h>


/* local defines */




int main()
{
	double	n, e, b ;


	n = 16.0 ;
	b = 2.0 ;
	e = log(n) / log(b) ;

	printf("answer= %12.4f\n",e) ;

	{
		double	hml, cp, p ;
		double	ccp, hmlc ;
		double	hml_f, hml_c ;


		p = 0.9 ;
		cp = ( 1.0 - p ) ;

		hml = log(cp) / log(p) ;

		hml_f = floor( log(cp) / log(p) ) ;

		printf("hml= %12.4f\n",hml) ;

		printf("hml_f= %12.4f\n",hml_f) ;

		ccp = pow(p,hml_f) ;

		printf("ccp_floor= %12.4f\n",ccp) ;

	}

	fclose(stdout) ;

	return 0 ;
}



