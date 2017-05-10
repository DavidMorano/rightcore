/* mainfibo */


/***************************************************************************

	This program calculates and prints out the first few
	Finonacci numbers.


***************************************************************************/


#include	<sys/types.h>
#include	<stdio.h>



#define	NN	20


extern unsigned int	fibonacci(unsigned int) ;



int main()
{
	unsigned int	i ;


	for (i = 0 ; i < NN ; i += 1) {

		fprintf(stdout," %u",
			fibonacci(i)) ;

	}

	fprintf(stdout,"\n") ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



