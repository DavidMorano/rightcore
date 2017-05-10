/* main (testctdecf) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This little program tests the 'ctdecf(3dam)' subroutine.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<ascii.h>


/* local defines */

#define	DBUFLEN		100
#define	NLOOPS		(16*1024*1024)


/* external subroutines */

extern int	ctdecf(char *,int,double,int,int,int,int) ;


/* forward references */


/* exported subroutines */


int main()
{
	double	v = 1.62 ;

	const int	dlen = DBUFLEN ;
	const int	fcode = 'f' ;
	const int	w = -1 ;
	const int	p = 3 ;
	const int	fill = -1 ;

	int	rs ;

	char	dbuf[DBUFLEN+1] ;


	rs = ctdecf(dbuf,dlen,v,fcode,w,p,fill) ;

	fprintf(stdout,"rs=%d s=>%s<\n",rs,dbuf) ;

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */



