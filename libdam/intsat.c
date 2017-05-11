/* intsat (Integer-Saturation) */

/* perform a variety of integer saturation addition-subtractions */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We provide some saturated add operations.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<localmisc.h>


/* exported subroutines */


int iaddsat(int v1,int v2)
{
	int		vr = (v1+v2) ;
	if (v1 >= 0) {
	    if (v2 >= 0) {
		if (vr < 0) vr = INT_MAX ;
	    }
	} else {
	    if (v2 < 0) {
		if (vr >= 0) vr = INT_MIN ;
	    }
	}
	return vr ;
}
/* end subroutine (iaddsat) */


long laddsat(long v1,long v2)
{
	long		vr = (v1+v2) ;
	if (v1 >= 0) {
	    if (v2 >= 0) {
		if (vr < 0) vr = LONG_MAX ;
	    }
	} else {
	    if (v2 < 0) {
		if (vr >= 0) vr = LONG_MIN ;
	    }
	}
	return vr ;
}
/* end subroutine (laddsat) */


long long lladdsat(long long v1,long long v2)
{
	long long	vr = (v1+v2) ;
	if (v1 >= 0) {
	    if (v2 >= 0) {
		if (vr < 0) vr = LONGLONG_MAX ;
	    }
	} else {
	    if (v2 < 0) {
		if (vr >= 0) vr = LONGLONG_MIN ;
	    }
	}
	return vr ;
}
/* end subroutine (lladdsat) */


uint uaddsat(uint v1,uint v2)
{
	const uint	m = (~ INT_MAX) ;
	uint		vr = (v1+v2) ;
	if ((v1&m) && (v2&m)) vr = UINT_MAX ;
	return vr ;
}
/* end subroutine (uaddsat) */


ulong uladdsat(ulong v1,ulong v2)
{
	const ulong	m = (~ LONG_MAX) ;
	ulong		vr = (v1+v2) ;
	if ((v1&m) && (v2&m)) vr = ULONG_MAX ;
	return vr ;
}
/* end subroutine (uladdsat) */


unsigned long long ulladdsat(unsigned long long v1,unsigned long long v2)
{
	const unsigned long long	m = (~ LONGLONG_MAX) ;
	unsigned long long		vr = (v1+v2) ;
	if ((v1&m) && (v2&m)) vr = ULONGLONG_MAX ;
	return vr ;
}
/* end subroutine (uladdsat) */


