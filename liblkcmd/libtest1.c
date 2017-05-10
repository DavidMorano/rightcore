
#include	<stdio.h>

#ifdef	COMMENT
void sub1(int v)
{
	printf("libtest1/sub1: v=%d\n",v) ;
	sub2(v) ;
}
#endif /* COMMENT */

void sub2(int v)
{
	printf("libtest1/sub2: v=%d\n",v) ;
}


