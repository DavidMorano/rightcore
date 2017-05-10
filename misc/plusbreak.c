/* plusbreak */

/* break up a line with plus signs in it */

/*
	David A.D. Morano
	May 1990
*/

/* 
	This program will break up a single line as produced by the
	output of 'i286size' into many lines broken up after each plus
	sign.

*/



#include	<fcntl.h>

#include	<bfile.h>

#include	"localmisc.h"





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errout, *efp = &errout ;

	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;

	unsigned int	ic ;

	int		rs ;


	if (bopen(efp,BERR,"w",0666) < 0) return BAD ;

	switch (argc) {

	case 0:
	case 1:
	    bopen(ifp,BIN,"r",0666) ;

	    bopen(ofp,BOUT,"w",0666) ;

	    break ;

	case 2:
	    rs = bopen(ifp,argv[1],"r",0666) ;

	    if (rs < 0) goto badin ;

	    rs = bopen(ofp,BOUT,"w",0666) ;

	    if (rs < 0) return rs ;

	    break ;

	case 3:
	default:
	    rs = bopen(ifp,argv[1],"r",0666) ;

	    if (rs < 0) goto badin ;

	    rs = bopen(ofp,argv[2],"wc",0666) ;

	    if (rs < 0) goto badout ;

	    break ;

	} /* end switch */



	while ((ic = bgetc(ifp)) != BEOF) {

		if (ic != '+') bputc(ofp,ic) ;

		else {

			bputc(ofp,'\n') ;

			bputc(ofp,ic) ;

		}
	}

	bclose(ofp) ;

exit:
	bclose(ifp) ;

badexit:
	bclose(efp) ;

	return OK ;

badin:
	bprintf(efp,"can't open input file (%d)\n",rs) ;

	goto badexit ;

badout:
	bprintf(efp,"can't open output file (%d)\n",rs) ;

	goto exit ;
}


