/* main */


#include	<envstandards.h>

#include	<stdio.h>

#include	<exitcodes.h>

#include	"defs.h"



int main() 
{
	FILE	*ifp = stdin ;
	FILE	*ofp = stdout ;

	int	rs ;
	int	len ;
	int	i ;
	int	ex = EX_OK ;

	char	linebuf[LINEBUFLEN + 1] ;


	while ((rs = ffread(ifp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    for (i = 0 ; i < len ; i += 1)
		linebuf[i] &= 0x7f ;

	    rs = ffwrite(ofp,linebuf,len) ;
	    if (rs < 0)
		break ;

	} /* end while */

	return ex ;
}
/* end subroutine (main) */



