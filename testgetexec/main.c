/* main (testgetexec) */



#include	<sys/types.h>
#include	<stdlib.h>
#include	<stdio.h>





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	char	*cp ;


	cp = argv[0] ;
	fprintf(stdout,"arg0=%s\n",
		(cp != NULL) ? cp : "*NA*") ;

	cp = getenv("_") ;

	fprintf(stdout,"_=%s\n",
		(cp != NULL) ? cp : "*NA*") ;

	cp = (char *) getexecname() ;

	fprintf(stdout,"execname=%s\n",
		(cp != NULL) ? cp : "*NA*") ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



