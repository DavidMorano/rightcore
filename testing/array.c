/* array */



#include	<bfile.h>

#include	"localmisc.h"



struct entry {
	int	i, j ;
} ;



int main()
{
	bfile	outfile, *ofp = &outfile ;

	struct entry	a[10][10] ;

	struct entry	*b ;

	int	i, j ;


	bopen(ofp,BFILE_STDOUT,"wct",0666) ;


	for (i = 0 ; i < 10 ; i += 1) {

	    for (j = 0 ; j < 10 ; j += 1) {

	        a[i][j].i = i ;
	        a[i][j].j = j ;

	    }

	}


	b = (struct entry *) &a[0][0] ;
	i = 0 ;
	while (i < 100) {

	    for (j = 0 ; j < 10 ; j += 1) {

	        bprintf(ofp," %d,%d",b[i].i,b[i].j) ;

	        i += 1 ;
	    }

	    bprintf(ofp,"\n") ;

	}

	bclose(ofp) ;

	return OK ;
}
/* end subroutine (main) */



