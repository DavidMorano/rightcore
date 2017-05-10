/* main (teststrtol) */


#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>



int main()
{
	long	val = 0 ;

	const char	*a = "  -21 b6" ;


	val = strtol(a,NULL,10) ;

	printf("val=%ld\n",val) ;

	return 0 ;
}
/* end subroutine (main) */


