/* main_struct */


/* revision history:

	= 2017-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<stdio.h>


struct flags {
	unsigned int	a : 1 ;
	unsigned int	b : 1 ;
	unsigned int	d : 1 ;
} ;


int main()
{
	struct flags	f ;


	printf("sizeof(struct flags)=%u\n",sizeof(struct flags)) ;

	return 0 ;
}


