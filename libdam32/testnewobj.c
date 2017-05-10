/* main (testnewobj) */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"newobj.h"

extern char	*strwcpy(char *,const char *,int) ;

int main()
{
	const int	n = 26 ;
	int		rs = SR_OK ;
	const char	*s = "hello world!" ;
	char		*a ;

	if ((a = newobj(char,(n+1))) != NULL) {
	    strwcpy(a,s,n) ;
	    printf("p=%p a=%s\n",a,a) ;
	    uc_free(a) ;
	} else
	    rs = SR_NOMEM ;

	printf("rs=%d\n",rs) ;

	return 0 ;
}
/* end subroutine (main) */


