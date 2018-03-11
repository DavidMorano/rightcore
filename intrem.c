/* intrem */

/* integer remainder */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutine calculates the unsigned remainder of a division of the
	two given numbers.

	Synopsis:

	int irem(v,m)
	int	v ;
	int	m ;

	Arguments:

	v	number to find the remainder for
	m	the modulus to use in the calculation

	Returns:

	-	the remainder value


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>		/* for shortened unsigned types */


/* exported subroutines */


int irem(int v,int m)
{
	const int	q = (v/m) ;
	return (v-(q*m)) ;
}
/* end subroutine (irem) */


long lrem(long v,long m)
{
	const long	q = (v/m) ;
	return (v-(q*m)) ;
}
/* end subroutine (lrem) */


LONG llrem(LONG v,int m)
{
	const LONG	q = (v/m) ;
	return (v-(q*m)) ;
}
/* end subroutine (llrem) */


uint iurem(uint v,int m)
{
	const int	q = (v/m) ;
	return (v-(q*m)) ;
}
/* end subroutine (iurem) */


ulong ulrem(ulong v,int m)
{
	const ulong	q = (v/m) ;
	return (v-(q*m)) ;
}
/* end subroutine (ulrem) */


ULONG ullrem(ULONG v,int m)
{
	const ULONG	q = (v/m) ;
	return (v-(q*m)) ;
}
/* end subroutine (ullrem) */


