/* ctoct */

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
#define	OURBASE		8


/* external subroutines */

extern int cvtdig(char *,int,ulonglong,int,int) ;


/* local structures */


/* forward references */

int	ctocti(char *,int,int) ;


/* local variables */


/* exported subroutines */


int ctoct(char *dp,int dl,int val)
{

	return ctocti(dp,dl,val) ;
}
/* end subroutine (ctoct) */


int ctocti(char *dp,int dl,int val)
{
	ulonglong	v = val ;
	const int	n = sizeof(int) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctocti) */


int ctoctui(char *dp,int dl,uint val)
{
	ulonglong	v = val ;
	const int	n = sizeof(uint) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctoctui) */


int ctoctl(char *dp,int dl,long val)
{
	ulonglong	v = val ;
	const int	n = sizeof(long) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctoctl) */


int ctoctul(char *dp,int dl,ulong val)
{
	ulonglong	v = val ;
	const int	n = sizeof(ulong) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctoctul) */


int ctoctll(char *dp,int dl,longlong val)
{
	ulonglong	v = val ;
	const int	n = sizeof(LONG) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctoctll) */


int ctoctull(char *dp,int dl,ulonglong val)
{
	ulonglong	v = val ;
	const int	n = sizeof(ULONG) ;

	return cvtdig(dp,dl,v,n,OURBASE) ;
}
/* end subroutine (ctoctull) */


