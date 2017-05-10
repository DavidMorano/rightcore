/* main (testsnloadavg) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little program tests the 'snloadavg(3dam)' subroutine.


*******************************************************************************/


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

extern int	snloadavg(char *,int,uint,int,int,int) ;
extern int	ctdecf(char *,int,double,int,int,int,int) ;


/* forward references */


/* exported subroutines */


int main()
{
	uint	la ;

	const int	dlen = DBUFLEN ;
	const int	fill = -1 ;

	int	rs ;
	int	w = -1 ;
	int	p = 3 ;

	char	dbuf[DBUFLEN+1] ;


	la = 0 ;
	la |= (3<<8) ;
	la |= 4 ;

	for (w = -1 ; w < 6 ; w += 1) {
	    for (p = -1 ; p < 4 ; p += 1) {
	        fprintf(stdout,"w=%d p=%d\n",w,p) ;
	        rs = snloadavg(dbuf,dlen,la,w,p,fill) ;
	        fprintf(stdout,"rs=%d s=>%s<\n",rs,dbuf) ;
	    } /* end for */
	} /* end for */

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */



