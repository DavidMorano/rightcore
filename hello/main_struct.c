/* main_struct */


#include	<stdio.h>


struct flags {
	unsigned int	a : 1 ;
	unsigned int	b : 1 ;
	unsigned int	d : 1 ;
} ;


int main()
{
	struct flags	f ;


	printf("sizeof(struct flags)=%u\n",sizeof(struct flags)) ;

	return 0 ;
}


