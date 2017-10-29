/* ctdecp */

/* subroutines to convert an integer to a decimal string /w precision */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine performs a conversion of an integer to its corresponding
        decimal string representation.

	Synopsis:

	int ctdecpX(rbuf,rlen,prec,v)
	char		rbuf[] ;
	int		rlen ;
	int		prec ;
	int		v ;

	Arguments:

	X		'i' or 'ui'
	rbuf		result buffer
	rlen		length of result buffer
	prec		precision
	v		value to convert

	Returns:

	>=0		length of result string
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern int	ctdeci(char *,int,int) ;
extern int	ctdecui(char *,int,uint) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

int		ctdecpui(char *,int,int,uint) ;

static int	zerofill(char *,int,int,int) ;


/* local variables */


/* exported subroutines */


int ctdecpi(char *rbuf,int rlen,int prec,int v)
{
	uint		uv = v ;
	int		rs ;
	int		rl = rlen ;
	int		n ;
	char		*rp = rbuf ;

	if (rbuf == NULL) return SR_FAULT ;

	if (v < 0) {
	    uv = -uv ;
	    rp += 1 ;
	    rl -= 1 ;
	}
	if ((rs = ctdecpui(rp,rl,prec,uv)) >= 0) {
	    n = rs ;
	    if (v < 0) {
	        rbuf[0] = '-' ;
	        n += 1 ;
	    }
	} /* end if (ctdecpio) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ctdecpi) */


int ctdecpui(char *rbuf,int rlen,int prec,uint uv)
{
	int		rs ;

	if (((rs = ctdecui(rbuf,rlen,uv)) >= 0) && (rs < prec)) {
	    rs = zerofill(rbuf,rlen,prec,rs) ;
	}

	return rs ;
}
/* end subroutine (ctdecpui) */


/* local subroutines */


static int zerofill(char *rbuf,int rlen,int prec,int n)
{
	int		rs = SR_OK ;
	const int	bi = (prec - n) ;

	if (prec <= rlen) {
	    int	i ;
	    for (i = (n-1) ; i >= 0 ; i -= 1) {
	       rbuf[i+bi] = rbuf[i] ;
	    }
	    for (i = 0 ; i < bi ; i += 1) {
		rbuf[i] = '0' ;
	    }
	    rs = prec ;
	} else {
	    rs = SR_OVERFLOW ;
	}

	return rs ;
}
/* end subroutine (zerofill) */


