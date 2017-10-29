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

extern int cvtdig(char *,ULONG,int,int) ;


/* local structures */


/* forward references */

int	ctocti(char *,int) ;


/* local variables */


/* exported subroutines */


int ctoct(char *buf,int val)
{

	return ctocti(buf,val) ;
}
/* end subroutine (ctoct) */


int ctocti(char *buf,int val)
{
	const int	n = sizeof(int) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (ctocti) */


int ctoctui(char *buf,uint val)
{
	const int	n = sizeof(uint) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (ctoctui) */


int ctoctl(char *buf,long val)
{
	const int	n = sizeof(long) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (ctoctl) */


int ctoctul(char *buf,ulong val)
{
	const int	n = sizeof(ulong) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (ctoctul) */


int ctoctll(char *buf,LONG val)
{
	const int	n = sizeof(LONG) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (ctoctll) */


int ctoctull(char *buf,ULONG val)
{
	const int	n = sizeof(ULONG) ;
	ULONG		v = val ;

	return cvtdig(buf,v,n,OURBASE) ;
}
/* end subroutine (ctoctull) */


