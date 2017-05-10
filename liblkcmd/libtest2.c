
#include	<stdio.h>

void	sub2(int) ;

void sub1(int v)
{
	printf("libtest2/sub1: v=%d\n",v) ;
	sub2(v) ;
}

void sub2(int v)
{
	printf("libtest2/sub2: v=%d\n",v) ;
}


