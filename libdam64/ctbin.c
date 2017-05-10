/* ctbin */

/* subroutines to convert an integer to a binary-digit string */


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
#define	OURBASE		2


/* external subroutines */

extern int cvtdig(char *,int,ulonglong,int,int) ;


/* local structures */


/* forward references */

int	ctbini(char *,int,int) ;


/* local variables */


/* exported subroutines */


int ctbin(char *dp,int dl,int val)
{

	return ctbini(dp,dl,val) ;
}
/* end subroutine (ctbin) */


int ctbini(char *dp,int dl,int val)
{
	ulonglong	v = val ;
	const int	n = sizeof(int) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctbini) */


int ctbinui(char *dp,int dl,uint val)
{
	ulonglong	v = val ;
	const int	n = sizeof(uint) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctbinui) */


int ctbinl(char *dp,int dl,long val)
{
	ulonglong	v = val ;
	const int	n = sizeof(long) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctbinl) */


int ctbinul(char *dp,int dl,ulong val)
{
	ulonglong	v = val ;
	const int	n = sizeof(ulong) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctbinul) */


int ctbinll(char *dp,int dl,longlong val)
{
	ulonglong	v = val ;
	const int	n = sizeof(LONG) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctbinll) */


int ctbinull(char *dp,int dl,ulonglong val)
{
	ulonglong	v = val ;
	const int	n = sizeof(ULONG) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctbinull) */


