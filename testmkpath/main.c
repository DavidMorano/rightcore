/* main */



#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdio.h>


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	char	buf[MAXPATHLEN + 1] ;


	buf[0] = '\0' ;
	if (argc > 1) {

	    switch (argc) {

	    case 2:
	        mkpath1(buf,argv[1]) ;

	        break ;

	    case 3:
	        mkpath2(buf,argv[1],argv[2]) ;

	        break ;

	    case 4:
	        mkpath3(buf,argv[1],argv[2],argv[3]) ;

	        break ;

	    case 5:
	        mkpath4(buf,argv[1],argv[2],argv[3],argv[4]) ;

	        break ;

	    } /* end switch */

	    fprintf(stdout,"%s\n",buf) ;

	} /* end if */

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



