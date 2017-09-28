/* main */

/* test compiler */

#include	<stdio.h>

struct test {
	int	a, b ;
} ;

static struct test	t ;

int main()
{

#ifdef	_GNUC_
	printf("_GNUC_ yes\n") ;
#else
	printf("_GNUC_ no\n") ;
#endif

	printf("a=%d\n",t.a) ;
	printf("b=%d\n",t.b) ;

	return 0 ;
}


