/* main (testsize) */


#include	<sys/types.h>
#include	<stdio.h>

#include	<localmisc.h>



struct testsize {
	uint	a ;
	uint	b : 4 ;
	uint	c : 1 ;
} ;



int main()
{


	printf("sizeof testsize=%d\n",sizeof(struct testsize)) ;

	fclose(stdout) ;

	return 0 ;
}



