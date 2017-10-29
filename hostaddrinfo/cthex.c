/* cthex */

/* subroutines to convert an integer to a HEX string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines that perform conversions which also have an integral
	power-of-two base are much faster than anything that uses 'lltostr(3c)'
	(or friends) because there are no division operations needed in the
	algorithm used.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* local defines */

#undef	OURBASE
#define	OURBASE		16


/* external subroutines */

extern int cvtdig(char *,ULONG,int,int) ;


/* local structures */


/* forward references */

int	cthexi(char *,int) ;


/* local variables */


/* exported subroutines */


int cthex(char *buf,int val)
{

	return cthexi(buf,val) ;
}
/* end subroutine (cthex) */


int cthexc(char *buf,int val)
{
	const int	n = sizeof(char) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (cthexc) */


int cthexuc(char *buf,uint val)
{
	const int	n = sizeof(uchar) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (cthexuc) */


int cthexs(char *buf,int val)
{
	const int	n = sizeof(short) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (cthexs) */


int cthexus(char *buf,uint val)
{
	const int	n = sizeof(ushort) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (cthexus) */


int cthexi(char *buf,int val)
{
	const int	n = sizeof(int) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (cthexi) */


int cthexui(char *buf,uint val)
{
	const int	n = sizeof(uint) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (cthexui) */


int cthexl(char *buf,long val)
{
	const int	n = sizeof(long) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (cthexl) */


int cthexul(char *buf,ulong val)
{
	const int	n = sizeof(ulong) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (cthexul) */


int cthexll(char *buf,LONG val)
{
	const int	n = sizeof(LONG) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (cthexll) */


int cthexull(char *buf,ULONG val)
{
	const int	n = sizeof(ULONG) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (cthexull) */


