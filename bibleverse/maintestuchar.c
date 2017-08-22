/* main (maintestuchar) */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<stdio.h>


int main()
{
	unsigned int	uv1, uv2, uv3 ;
	int		v1, v2, v3 ;
	unsigned char	uch = '°' ;
	signed char	sch = '°' ;
	char		ch = '°' ;


	printf("pv1=%d pv2=%d pv3=%d\n",uch,sch,ch) ;

	v1 = uch ;
	v2 = sch ;
	v3 = ch ;
	printf("v1=%d v2=%d v3=%d\n",v1,v2,v3) ;

	uv1 = uch ;
	uv2 = sch ;
	uv3 = ch ;
	printf("uv1=%d uv2=%d uv3=%d\n",uv1,uv2,uv3) ;

	return 0 ;
}


