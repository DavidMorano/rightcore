/* intrem */

/* integer remainder */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutine calculates the unsigned remainder of a fivision of the
	two given number.

	Synopsis:

	int irem(v,m)
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


LONG llrem(LONG v,LONG m)
{
	const LONG	q = (v/m) ;
	return (v-(q*m)) ;
}
/* end subroutine (llrem) */


uint uceil(uint v,uint m)
{
	const uint	q = (v/m) ;
	return (v-(q*m)) ;
}
/* end subroutine (uceil) */


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


