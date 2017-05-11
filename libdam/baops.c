/* baops */

/* bit-array operations */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines are cousins to the BAOPS macros.  Except that the
	"set" and "clr" versions of these subroutines return the previous bit
	state whereas the BAOPS macro versions do not.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>

#include	"baops.h"


/* exported subroutines */


int baset(uchar *a,int n)
{
	int	rs = BATST(a,n) ;
	BASET(a,n) ;
	return rs ;
}
/* end subroutine (baset) */


int baclr(uchar *a,int n)
{
	int	rs = BATST(a,n) ;
	BACLR(a,n) ;
	return rs ;
}
/* end subroutine (baclr) */


int batst(uchar *a,int n)
{
	return BATST(a,n) ;
}
/* end subroutine (batst) */


int basetl(ULONG *a,int n)
{
	int	rs = BATSTL(a,n) ;
	BASETL(a,n) ;
	return rs ;
}
/* end subroutine (basetl) */


int baclrl(ULONG *a,int n)
{
	int	rs = BATSTL(a,n) ;
	BACLRL(a,n) ;
	return rs ;
}
/* end subroutine (baclrl) */


int batstl(ULONG *a,int n)
{
	return BATSTL(a,n) ;
}
/* end subroutine (batstl) */


