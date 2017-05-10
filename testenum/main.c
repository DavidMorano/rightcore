/* main (testenum) */



#include	<stdio.h>




enum keys {
	key_zero,
	key_one,
	key_two,
	key_overlast
} ;


static int sub(enum keys) ;




int main()
{
	enum keys	a ;

	int		i ;


	printf("sizeof=%d\n",sizeof(enum keys)) ;

	a = key_one ;
	printf("a=%d zero=%d\n",a,key_zero) ;

	i = key_two ;
	printf("i=%d\n",i) ;

	sub(key_two) ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int sub(n)
enum keys	n ;
{


	printf("sub: n=%d\n",n) ;

}
/* end subroutine (sub) */



