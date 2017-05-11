/* intfloor */

/* integer floor */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Implementtion note:

	Note that a "sequence point" needs to be inserted into this code so
	that the total expression doesn't get optimized out!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int ifloor(int v,int m)
{
	const int	n = v / m ;
	return (n * m) ;
}
/* end subroutine (ifloor) */


long lfloor(long v,int m)
{
	const long	n = v / m ;
	return (n * m) ;
}
/* end subroutine (lfloor) */


LONG llfloor(LONG v,int m)
{
	const LONG	n = v / m ;
	return (n * m) ;
}
/* end subroutine (llfloor) */


uint ufloor(uint v,int m)
{
	const uint	n = v / m ;
	return (n * m) ;
}
/* end subroutine (ufloor) */


ulong ulfloor(ulong v,int m)
{
	const ulong	n = v / m ;
	return (n * m) ;
}
/* end subroutine (ulfloor) */


ULONG ullfloor(ULONG v,int m)
{
	const ulong	n = v / m ;
	return (n * m) ;
}
/* end subroutine (ullfloor) */


