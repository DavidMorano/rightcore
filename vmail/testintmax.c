/* testintmax */
/* lang=C99 */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<stdio.h>
extern int	ndigits(int,int) ;
int main()
{
	int	n ;
	int	v = INT_MAX ;

	n = ndigits(v,10) ;

	printf("v=%d n=%u\n",v,n) ;

	return 0 ;
}


