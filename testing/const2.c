/* main (const2) */



#include	<sys/types.h>
#include	<stdio.h>

#include	"localmisc.h"



static char	*b = "hello world" ;




int main()
{


	fprintf(stdout,"b=%p\n",b) ;

	fclose(stdout) ;

	return 0 ;
}


