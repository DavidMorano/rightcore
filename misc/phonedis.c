/* program to compute the allowable phone discount */


#include	<stdio.h>


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	double		tcharges, dcharges ;


	printf("enter total AT&T charges including tax\n") ;

	while (scanf("%lf",&tcharges) != EOF) {

		if (tcharges > 100.00) tcharges = 100.00 ;

		dcharges = tcharges ;

		if (tcharges > 35.00) 
			dcharges = 35.00 + ((tcharges - 35.00) * 0.5) ;

		printf("allowable discount is %6.2f\n",dcharges) ;

	} ; /* end while */

	return (0) ;
}


