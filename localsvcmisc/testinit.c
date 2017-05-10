/* testinit (C89) */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<stdio.h>
struct multi {
	int	a ;
	int	b ;
} ;
	struct multi	aa = { 1 } ;
int main(int argc,const char **argv,const char **envv)
{


	printf("aa.a=%08lx aa.b=%08lx\n",aa.a,aa.b) ;

	return 0 ;
}
/* end subroutine (main) */

