/* main (const) */



#include	<sys/types.h>
#include	<stdio.h>

#include	"localmisc.h"



static const char	*a[] = {
	"hello",
	"goodbye",
	NULL
} ;




int main()
{


	fprintf(stdout,"a=%p\n",a) ;

	fprintf(stdout,"a[0]=%p\n",a[0]) ;

	fclose(stdout) ;

	return 0 ;
}



