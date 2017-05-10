/* main (testvla) */

#include	<stdio.h>

#define	NA	10

/* forward */

static int	sub(int [],int) ;

int main()
{
	int	a[NA + 1] ;
	int	i ;
	int	sum ;

	for (i = 0 ; i < NA ; i += 1) {
		a[i] = i ;
	}

	a[i] = 0 ;

	sum = sub(a,NA) ;

	printf("sum=%u\n",sum) ;

	return 0 ;
}


int sub(a,n)
int	n ;
int	a[n] ;
{
	int	sum = 0 ;
	int	i ;
	int	as[n + 1] ;


	for (i = 0 ; i < n ; i += 1) {

		as[i] = a[i] ;
		if (a[i] > 0)
			as[i] -= 1 ;

	}

	for (i = 0 ; i < n ; i += 1) {

		sum += a[i] ;
		printf("a[%u]=%u\n",i,as[i]) ;

	}

	return sum ;
}


