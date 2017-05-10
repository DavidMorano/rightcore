/* testema */

#include	<envstandards.h>

#include	<stdio.h>

#include	"comparse.h"


#define	CBUFLEN	100


int main()
{
	FILE	*ofp = stdout ;

	COMPARSE	c ;

	int	rs1 = 0 ;

	const char	*ts = " \"26\\\"23\" (content type) more here" ;


	if ((rs1 = comparse_start(&c,ts,-1)) >= 0) {

	fprintf(ofp,"comparse_load() rs=%d\n",rs1) ;
	fprintf(ofp,"val=>%s<\n",c.value) ;
	fprintf(ofp,"com=>%s<\n",c.comment) ;

	    comparse_finish(&c) ;
	} /* end if */

	return 0 ;
}
/* end subroutine (main) */


