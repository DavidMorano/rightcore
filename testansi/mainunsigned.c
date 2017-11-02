/* mainunsigned */

/* we test (what) some unsigned operations? */

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
	uint		uch ;
	int		ch = 0 ;

	uch = 'a' ;
	printf("ch=%d\n",uch) ;

	uch = 42U ;
	printf("ch=%d\n",uch) ;

	return 0 ;
}
/* end subroutine (mainunsigned) */


/* local subroutines */


