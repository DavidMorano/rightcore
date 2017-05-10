/* strip page control */


/* 
	Strip the form feeds out of the file.

*/



#include	"rel.h"

#include	<fcntl.h>

#include	"bfile.h"

#include	<stdio.h>




#define		BUFL	2048



	extern int	bopen(), bclose(), bflush() ;
	extern int	bprintf(), bgetc(), bputc() ;



int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		input, *ifp, output, *ofp ;


	register int	c, i, n ;

	int		offset, page, line, rs ;



	ifp = &input ;
	ofp = &output ;


	switch (argc) {

case 1:
	bopen(ifp,BFILE_IN,"r",0666) ;

	bopen(ofp,BFILE_OUT,"w",0666) ;

	break ;

case 2:
	rs = bopen(ifp,argv[1],"r",0666) ;
	if (rs < 0) return (rs) ;

	rs = bopen(ofp,BFILE_OUT,"w",0666) ;
	if (rs < 0) return rs ;

	break ;

case 3:
default:
	rs = bopen(ifp,argv[1],"r",0666) ;
	if (rs < 0) return (rs) ;

	rs = bopen(ofp,argv[2],"w",0666) ;
	if (rs < 0) return rs ;

	break ;

	} ; /* end switch */



	while ((c = bgetc(ifp)) != BFILE_EOF) {

		if (c == '\014') {

			bputc(ofp,'\n') ;

		} else bputc(ofp,c) ;

	} ;


	bflush(ofp) ;

	bclose(ofp) ;

}


