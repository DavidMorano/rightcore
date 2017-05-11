/* intceil */

/* integer ceiling */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine calculates the unsigned ceiling of a number given a
	specified modulus.

	Synopsis:

	int iceil(v,m)
	int	v ;
	int	m ;

	Arguments:

	v	number to find the ceiling for
	m	the modulus to use in the calculation

	Returns:

	-	the ceiling value

	Implementtion note:

        Note that a "sequence point" needs to be present in this code so that
        the total expression doesn't get optimized out!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* exported subroutines */


int iceil(int v,int m)
{
	const int	n = (v + (m - 1)) / m ;
	return (n * m) ;
}
/* end subroutine (iceil) */


long lceil(long v,int m)
{
	const long	n = (v + (m - 1)) / m ;
	return (n * m) ;
}
/* end subroutine (lceil) */


LONG llceil(LONG v,int m)
{
	const LONG	n = (v + (m - 1)) / m ;
	return (n * m) ;
}
/* end subroutine (llceil) */


uint uceil(uint v,int m)
{
	const uint	n = (v + (m - 1)) / m ;
	return (n * m) ;
}
/* end subroutine (uceil) */


ulong ulceil(ulong v,int m)
{
	const ulong	n = (v + (m - 1)) / m ;
	return (n * m) ;
}
/* end subroutine (ulceil) */


ULONG ullceil(ULONG v,int m)
{
	const ULONG	n = (v + (m - 1)) / m ;
	return (n * m) ;
}
/* end subroutine (ullceil) */


