/* mapex */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<sys/types.h>

#include	<exitcodes.h>
#include	<localmisc.h>


/* exported subroutines */


int mapex(const struct mapex mapexs[],int rs)
{
	int		i, ex ;

	for (i = 0 ; mapexs[i].rs != 0 ; i += 1) {
	    if (mapexs[i].rs == rs) break ;
	} /* end for */

	ex = (mapexs[i].rs != 0) ? mapexs[i].ex : EX_UNKNOWN ;

	return ex ;
}
/* end subroutine (mapex) */


