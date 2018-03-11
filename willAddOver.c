/* willAddOver */


/* revision history:

	= 2012-11-21, David A­D­ Morano
	I took this from some of my previous code.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<localmisc.h>


int willAddOver(long n1,long n2)
{
	int	f = FALSE ;
	f = f || (n1 > 0) && (n2 > 0) && (n1 > (LONG_MAX - n2)) ;
	f = f || (n1 < 0) && (n2 < 0) && (n1 < (LONG_MIN - n2)) ;
	return f ;
}
/* end subroutine (willAddOver) */


