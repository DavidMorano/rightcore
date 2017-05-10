/* main */



#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>



int main()
{
	int	i, ch ;


	for (i = 0 ; i < 256 ; i += 1) {

		if (ispunct(i))
			fprintf(stdout,"%02x %c\n",i,i) ;

	} /* end for */

	fclose(stdout) ;

	return 0 ;
}


