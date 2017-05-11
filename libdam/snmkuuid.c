/* snmkuuid */

/* string-UUID (String-UUID) */


#define	CF_DEBUGS	0		/* conmpile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a string (in the standard representation) from
	a UUID.

	Synopsis:

	int snmkuuid(rbuf,rlen,up)
	char		rbuf[] ;
	int		rlen ;
	MKUUID		*up ;

	Arguments:

	rbuf		result buffer
	rlen		size of supplied result buffer
	up		pointer to MKUUID object holding a UUID

	Returns:

	>=0		resulting length
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<cthex.h>
#include	<mkuuid.h>
#include	<localmisc.h>


/* local defines */

#define	DBUFLEN		16


/* external subroutines */


/* forward references */

static int sbuf_hexp(SBUF *,uint64_t,int) ;


/* exported subroutines */


int snmkuuid(char *dbuf,int dlen,MKUUID *up)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;

	if (dlen == NULL) return SR_FAULT ;
	if (up == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&b,dbuf,dlen)) >= 0) {
	    uint64_t	v ;

	    v = (up->time & UINT_MAX) ;
	    if (rs >= 0) rs = sbuf_hexp(&b,v,4) ;

	    if (rs >= 0) rs = sbuf_char(&b,'-') ;

	    v = ((up->time >> 32) & UINT_MAX) ;
	    if (rs >= 0) rs = sbuf_hexp(&b,v,2) ;

	    if (rs >= 0) rs = sbuf_char(&b,'-') ;

	    if (rs >= 0) {
		uint64_t	tv ;
		v = 0 ;
		tv = ((up->time >> 48) & UINT_MAX) ;
		v |= (tv & UCHAR_MAX) ;
		tv = (up->version & (16-1)) ;
		v |= (tv << 8) ;
		tv = ((up->time >> 48) & UINT_MAX) ;
		v |= (tv << (8+4)) ;
	        rs = sbuf_hexp(&b,v,2) ;
	    }

	    if (rs >= 0) rs = sbuf_char(&b,'-') ;

	    v = (up->clk & UINT_MAX) ;
	    v &= (~ (3 << 14)) ;
	    v |= (2 << 14) ;		/* standardized variant */
	    if (rs >= 0) rs = sbuf_hexp(&b,v,2) ;

	    if (rs >= 0) rs = sbuf_char(&b,'-') ;

	    v = up->node ;
	    if (rs >= 0) rs = sbuf_hexp(&b,v,6) ;

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (snmkuuid) */


static int sbuf_hexp(SBUF *bp,uint64_t v,int n)
{
	uint		vi ;
	const int	dlen = DBUFLEN ;
	int		rs ;
	char		dbuf[DBUFLEN+1] ;

	switch (n) {
	case 2:
	    vi = (uint) v ;
	    rs = cthexus(dbuf,dlen,vi) ;
	    break ;
	case 4:
	    vi = (uint) v ;
	    rs = cthexui(dbuf,dlen,vi) ;
	    break ;
	case 6:
	    rs = cthexull(dbuf,dlen,v) ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

	if (rs >= 0) {
	    const char	*dp = dbuf ;
	    if (n == 6) dp += ((8-n)*2) ;
	    rs = sbuf_strw(bp,dp,(n*2)) ;
	} /* end if */

	return rs ;
}
/* end subroutine (sbuf_hexp) */


