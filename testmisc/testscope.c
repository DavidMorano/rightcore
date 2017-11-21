/* testscope */

#include	<stdio.h>

static int	a ;

static void foo(void) ;

int main()
{
	foo() ;
	foo() ;
	return 0 ;
}

static void foo(void) {
	static int	a = 0 ;
	a += 1 ;
	printf("a=%u\n",a) ;
}


