/* bbfile */

/* convert arbitrary string to standard file name */



#include	<sys/param.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<stdio.h>


#define	BB_MAXNAMELEN	14



int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	int i,j,k ;


	k = 0 ;
	for (i = 1; i < argc; i += 1) {

	    j = 0 ;
	    while (argv[i][j] != '\0') {

	        if (isalnum(argv[i][j]) ||
	            argv[i][j] == '+' ||
	            argv[i][j] == '_' ||
	            argv[i][j] == '-' ||
	            argv[i][j] == '.' ) {

	            putc(argv[i][j],stdout) ;

	            if (++k == BB_MAXNAMELEN) goto done ;

	        }
	        j++ ;
	    }

	    if (++k == BB_MAXNAMELEN) break ;

	    if ((i+1) == argc) break ;

	    putc('_',stdout) ;

	} /* end for */

done:
	putc('\n',stdout) ;

	fflush(stdout) ;

	fflush(stderr) ;

	return 0 ;
}
/* end subroutine (main) */


