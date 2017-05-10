/* generate line numbers on a text file */


#include	"bfile.h"

#include	"localmisc.h"



/* local defines */

#define		LINESIZE	200




int main()
{
	bfile	in, *ifp = &in ;
	bfile	out, *ofp = &out ;

	int	len, c ;

	char	buf[LINESIZE + 1] ;


	bopen(ifp,BIN,"r") ;

	bopen(ofp,BOUT,"wct") ;

	c = 0 ;
	while ((len = breadline(ifp,buf,LINESIZE)) >= 0) {

		bprintf(ofp,"%02d	%W",c,buf,len) ;

		c += 1 ;
	}

	bclose(ofp) ;

	bclose(ifp) ;

	return OK ;
}
/* end subroutine (main) */



