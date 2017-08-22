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

extern int cvtdig(char *,int,ulonglong,int,int) ;


/* local structures */


/* forward references */

int	cthexi(char *,int,int) ;


/* local variables */


/* exported subroutines */


/* integer */
int cthex(char *dp,int dl,int val)
{

	return cthexi(dp,dl,val) ;
}
/* end subroutine (cthex) */


/* character */
int cthexc(char *dp,int dl,int val)
{
	ulonglong	v = val ;
	const int	n = sizeof(char) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (cthexc) */


/* unsigned character */
int cthexuc(char *dp,int dl,uint val)
{
	ulonglong	v = val ;
	const int	n = sizeof(uchar) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (cthexuc) */


/* short */
int cthexs(char *dp,int dl,int val)
{
	ulonglong	v = val ;
	const int	n = sizeof(short) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (cthexs) */


/* unsigned short */
int cthexus(char *dp,int dl,uint val)
{
	ulonglong	v = val ;
	const int	n = sizeof(ushort) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (cthexus) */


/* integer */
int cthexi(char *dp,int dl,int val)
{
	ulonglong	v = val ;
	const int	n = sizeof(int) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (cthexi) */


/* unsigned integer */
int cthexui(char *dp,int dl,uint val)
{
	ulonglong	v = val ;
	const int	n = sizeof(uint) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (cthexui) */


/* long */
int cthexl(char *dp,int dl,long val)
{
	ulonglong	v = val ;
	const int	n = sizeof(long) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (cthexl) */


/* unsigned long */
int cthexul(char *dp,int dl,ulong val)
{
	ulonglong	v = val ;
	const int	n = sizeof(ulong) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (cthexul) */


/* long-long */
int cthexll(char *dp,int dl,longlong val)
{
	ulonglong	v = val ;
	const int	n = sizeof(LONG) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (cthexll) */


/* unsigned long-long */
int cthexull(char *dp,int dl,ulonglong val)
{
	ulonglong	v = val ;
	const int	n = sizeof(ULONG) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (cthexull) */


