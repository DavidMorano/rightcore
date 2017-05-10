/* main (textcvt) */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<stdio.h>
#include	<ctbin.h>
#include	<ctoct.h>
#include	<cthex.h>
#include	<localmisc.h>


#deifne	OBUFLEN	100


int main()
{
	const int	olen = OBUFLEN ;
	const int	val = 0x80000000 ;
	int		n ;
	char		obuf[OBUFLEN+1] ;


	n = ctbin(obuf,olen,val) ;
	printf("n=%d bin=%s\n",n,obuf) ;

	n = ctoct(obuf,olen,val) ;
	printf("n=%d oct=%s\n",n,obuf) ;

	n = cthex(obuf,olen,val) ;
	printf("n=%d hex=%s\n",n,obuf) ;

	return 0 ;
}
/* end subroutine (main) */


