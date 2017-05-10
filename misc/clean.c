

#include	"localmisc.h"

#include	"bfile.h"

main()
{
	bfile	in, *ifp = &in ;
	bfile	out, *ofp = &out ;
	bfile	err, *efp = &err ;

	int	c ;


	bopen(efp,2,"w",0666) ;

	bopen(ifp,0,"r",0666) ;

	bopen(ofp,1,"w",0666) ;


	while ((c = bgetc(ifp)) != BEOF) {

		if (c != ' ' && c != '\t') bputc(ofp,c) ;

	} ;


	bclose(ofp) ; 

	bclose(efp) ;

	return OK ;
}


