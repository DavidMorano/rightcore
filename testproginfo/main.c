/* main */


#include	<sys/types.h>
#include	<stdlib.h>
#include	<stdio.h>



int main(argc,argv,envv)
char	argc ;
char	*argv[] ;
char	*envv[] ;
{
	int	rs ;

	char	*cp ;



	fprintf(stdout,"argname=%s\n",
		((argc > 0) ? argv[0] : "")) ;

	cp = getenv("_") ;

	fprintf(stdout,"prgfname=%s\n",
		((cp != NULL) ? cp : "")) ;


	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



