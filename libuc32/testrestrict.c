/* testrestrict */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<stdio.h>

#ifdef	__GNUC__
#define	restrict	__restrict
#else
#define	restrict
#endif

static int sub(int *,int *,int) ;

int main()
{
	const int	n = 4 ;
	int		a[4] = { 0, 1, 2, 3 } ;
	int		b[4] = { 4, 5, 6, 7 } ;
	sub(a,b,n) ;
	printf("answer is %d\n",a[3]) ;
	return 0 ;
}
static int sub(int *restrict ap,int *restrict bp,int n)
{
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    bp[i] = ap[i] ;
	}
	return n ;
}

