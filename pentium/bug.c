

#include <stdio.h>
 


/* forward references */

double	sub() ;


/* local variables */

static double	test1[] = {
        4195835.0,
        4195835.0,
	3145727.0,
	3145727.0,
} ;

static double	test2[] = {
        824633702441.0,
	1.0,
	824633702441.0,
} ;

double	temp ;


 
main()
{
	double	a, b ;
	double	r ;


	a = sub(test1[1] / test1[2]) ;

	b = sub(a * test1[3]) ;

	r = sub(b - test1[0]) ;
	 
        printf("answer should be zero --> %20.12f\n", r);
 

	a = test2[1] / test2[2] ;
        r = test2[0] * a ;

        printf("answer should be one  --> %20.12f\n", r);

	printf("if either is incorrect, then you have the Pentium bug\n") ;

        return 0 ;
}


double sub(a)
double	a ;
{

	temp = a ;
	return temp ;
}



