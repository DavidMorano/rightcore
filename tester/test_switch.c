/* main */


#include	<math.h>

#include	<bfile.h>




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	outfile, *ofp = &outfile ;

	int	i ;




	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;


	for (i = 0 ; i < 3 ; i += 1) {

	switch (i) {

	case 0:
		bprintf(ofp,"got a 0\n") ;

		break ;

	case 2:
		bprintf(ofp,"got a 2\n") ;

		break ;

	} /* end switch */

	} /* end for */

	bclose(ofp) ;

	return 0 ;
}



