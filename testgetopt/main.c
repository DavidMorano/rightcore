

#include	<sys/types.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	"localmisc.h"



extern int	optind ;

extern char	*optargs ;




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	int	opt ;
	int	i, p ;
	int	f_a = FALSE ;
	int	f_b = FALSE ;
	int	f_c = FALSE ;
	int	f_bad = FALSE ;
	int	f_default = FALSE ;

	char	*va, *vb ;


	while ((opt = getopt(argc,argv,"a:b:c:")) != EOF) {

		switch (opt) {

		case 'a':
			f_a = TRUE ;
			va = optarg ;
			break ;

		case 'b':
			f_b = TRUE ;
			vb = optarg ;
			break ;

		case 'c':
			f_a = TRUE ;
			break ;

		case '?':
			f_bad = TRUE ;
			break ;

		default:
			f_default = TRUE ;
			break ;

		} /* end switch */

	} /* end while */

	printf("f_a=%d v=%s\n",f_a,(f_a) ? va : "") ;

	printf("f_b=%d v=%s\n",f_b,(f_b) ? vb : "") ;

	printf("f_c=%d \n",f_a) ;

	printf("f_bad=%d \n",f_bad) ;

	printf("f_default=%d \n",f_default) ;

	p = 0 ;
	for (i = optind ; i < argc ; i += 1) {

		printf("p[%d]=%s\n",p,argv[i]) ;

		p += 1 ;

	} /* end for */

	fclose(stdout) ;

	return 0 ;
}


