/* snloadavg */

/* make string version of a load-average */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Ths subroutine creates a string that represents a load-average value.
	Load-averages -- in their native form -- are fixed-point numbers with a
	24-bit integer part and an 8-bit fractional part.

	Synopsis:

	int snloadavg(dbuf,dlen,la,w,p,fill)
	char		*dbuf ;
	int		dlen ;
	uint		la ;
	int		w ;
	int		p ;
	int		fill ;
	
	Arguments:

	dbuf		destination string buffer
	dlen		destination string buffer length
	la		load-average (32-bit unsigned fixed-point)
	w		width
	p		precision
	fill		fill indicator

	Returns:

	>=0		number of bytes in result
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern int	ctdecpui(char *,int,int,uint) ;
extern int	ctdecui(char *,int,uint) ;


/* external variables */


/* local structures */

struct parts {
	uint	parti ;
	uint	partf ;
} ;


/* forward references */

static int	parts_load(struct parts *,uint) ;
static int	parts_round(struct parts *,int) ;


/* local variables */


/* exported subroutines */


int snloadavg(char *dbuf,int dlen,uint la,int w,int p,int fill)
{
	struct parts	partla, *pp = &partla ;
	const int	diglen = DIGBUFLEN ;
	int		rs = SR_OK ;
	int		zfprec ;
	int		dl ;
	int		prec = -1 ;
	int		i = 0 ;
	const char	*dp ;
	char		digbuf[DIGBUFLEN+1] ;

	if (dbuf == NULL) return SR_FAULT ;

	if (p > 3) p = 3 ;

/* calculate parts */

	parts_load(pp,la) ;

/* round out the value according to the specified precision */

	parts_round(pp,p) ;

/* calculate some parameters */

	if (w >= 0) {
	    if (p > 0) {
	        prec = (w - 1 - p) ;
	    } else {
	        prec = w ;
	    }
	    if (prec < 0) prec = 0 ;
	}

/* put the resulting string together */

	zfprec = (fill == 0) ? prec : 0 ;
	dp = digbuf ;
	if ((rs = ctdecpui(digbuf,diglen,zfprec,pp->parti)) >= 0) {
	    dl = rs ;
	    if ((prec >= 0) && (prec < dl)) {
	        dp += (dl-prec) ;
	        dl = prec ;
	    }
	    rs = storebuf_strw(dbuf,dlen,i,dp,dl) ;
	    i += rs ;
	}

	if ((rs >= 0) && (p >= 0)) {

	    rs = storebuf_char(dbuf,dlen,i,'.') ;
	    i += rs ;

	    if ((rs >= 0) && (p > 0)) {
	        if ((rs = ctdecpui(digbuf,diglen,p,pp->partf)) >= 0) {
	            dl = rs ;
	            if (dl > p) dl = p ;
	            rs = storebuf_strw(dbuf,dlen,i,digbuf,dl) ;
	            i += rs ;
	        }
	    }

	} /* end if */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (snloadavg) */


/* local subroutines */


static int parts_load(struct parts *pp,uint la)
{
	uint		partf ;
	partf = (la & (FSCALE-1)) ;
	partf = (partf * 1000) ;
	partf = (partf / FSCALE) ;
	pp->parti = (la >> FSHIFT) ;
	pp->partf = partf ;
	return 0 ;
}
/* end subroutine (parts_load) */


static int parts_round(struct parts *pp,int prec)
{
	int		r ;
	switch (prec) {
	case 3: /* no change needed */
	    break ;
	case 2:
	    r = (pp->partf%10) ;
	    if (r >= 5) pp->partf += (10-r) ;
	    break ;
	case 1:
	    r = (pp->partf%100) ;
	    if (r >= 50) pp->partf += (100-r) ;
	    break ;
	case 0:
	    r = pp->partf ;
	    if (r >= 500) pp->partf += (1000-r) ;
	    break ;
	} /* end switch */
	if (pp->partf >= 1000) {
	    pp->partf = (pp->partf % 1000) ;
	    pp->parti += 1 ;
	}
	return 0 ;
}
/* end subroutine (parts_round) */


