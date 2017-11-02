/* mainmod */

/* we test (what) modulas operations? */

#define	CF_DEBUGS		0	/* compile-time debugging */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<localmisc.h>


/* external subroutines */


/* forward references */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	const int	n = (10*1024*1024) ;
	int		rv = 0 ;
	int		r = 0 ;
	int		i ;

	for (i = 0 ; i < n ; i += 1) {
	    rv = random() ;
	    r = (rv%1) ;
	    if (r != 0) break ;
	}
	printf("rv=%d r=%d\n",rv,r) ;

	return 0 ;
}
/* end subroutine (mainmod) */


/* local subroutines */


