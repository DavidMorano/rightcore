/* testhdrextnum */
/* lang=C99 */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<stdio.h>


int main()
{
	FILE		*fp = stdout ;
	int		v ;
	int		sl = -1 ;
	const char	*sp = "(this) 8 (is the thing)" ;

	v = hdrextnum(sp,sl) ;

	fprintf(fp,"v=%d\n",v) ;
	fflush(fp) ;

	return 0 ;
}
/* end subroutine (main) */


