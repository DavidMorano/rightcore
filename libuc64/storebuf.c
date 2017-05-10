/* storebuf */

/* storage buffer manipulation subroutines */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine can be used to construct strings or messages in a buffer
        WITHOUT using the 'sprintf(3c)' subroutine.

        This module is useful when the user supplies a buffer of a specified
        length and does not want to track the creation and destruction of an
        associated object. There is NO object (besides the user supplied buffer
        -- which can be considered THE object) to create and then destroy when
        using this module.

	The user must carefully track the buffer usage so that subsequent calls
	can be supplied with the correct index value of the next available
	(unused) byte in the buffer.

	Example usage:

	rs = 0 ;
	i = 0 ;
	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,sp,sl) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_buf(rbuf,rlen,i,bp,bl) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_deci(rbuf,rlen,i,value) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,sp,sl) ;
	    i += rs ;
	}


*******************************************************************************/


#define	STOREBUF_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ctdec.h>
#include	<cthex.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	HEXBUFLEN
#define	HEXBUFLEN	32		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


int storebuf_char(char *rbuf,int rlen,int i,int ch)
{
	int		rs = SR_OK ;
	char		*bp = (rbuf + i) ;

	if (i < 0)
	    return SR_INVALID ;

	if ((rlen < 0) || ((rlen - i) >= 1)) {
	    *bp++ = ch ;
	} else
	    rs = SR_OVERFLOW ;

	*bp = '\0' ;
	return (rs >= 0) ? 1 : rs ;
}
/* end subroutine (storebuf_char) */


int storebuf_buf(char *rbuf,int rlen,int i,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	char		*bp = (rbuf + i) ;

	if (i < 0)
	    return SR_INVALID ;

	if (rlen < 0) {
	    if (sl < 0) {
	        while (*sp) {
	            *bp++ = *sp++ ;
	 	}
	    } else {
	        memcpy(bp,sp,sl) ;
	        bp += sl ;
	    } /* end if */
	} else {
	    if (sl < 0) {
	        while ((bp < (rbuf + rlen)) && *sp) {
	            *bp++ = *sp++ ;
		}
		if ((bp == (rbuf + rlen)) && (*sp != '\0')) {
		    rs = SR_OVERFLOW ;
		}
	    } else {
		if ((rlen - i) >= sl) {
	            memcpy(bp,sp,sl) ;
	            bp += sl ;
		} else
		    rs = SR_OVERFLOW ;
	    } /* end if */
	} /* end if */

	*bp = '\0' ;
	return (rs >= 0) ? (bp - (rbuf + i)) : rs ;
}
/* end subroutine (storebuf_buf) */


int storebuf_strw(char *rbuf,int rlen,int i,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	char		*bp = (rbuf + i) ;

	if (i < 0)
	    return SR_INVALID ;

	if (rlen < 0) {
	    if (sl < 0) {
	        while (*sp) {
	            *bp++ = *sp++ ;
		}
	    } else {
	        while (sl && *sp) {
	            *bp++ = *sp++ ;
	            sl -= 1 ;
	        }
	    } /* end if */
	} else {
	    if (sl < 0) {
	        while ((bp < (rbuf + rlen)) && *sp) {
	            *bp++ = *sp++ ;
		}
		if ((bp == (rbuf + rlen)) && (*sp != '\0')) {
		    rs = SR_OVERFLOW ;
		}
	    } else {
	        while ((bp < (rbuf + rlen)) && sl && *sp) {
	            *bp++ = *sp++ ;
	            sl -= 1 ;
	        }
		if ((bp == (rbuf + rlen)) && sl) {
		    rs = SR_OVERFLOW ;
		}
	    } /* end if */
	} /* end if */

	*bp = '\0' ;
	return (rs >= 0) ? (bp - (rbuf + i)) : rs ;
}
/* end subroutine (storebuf_strw) */


int storebuf_deci(char *rbuf,int rlen,int i,int v)
{
	const int	dlen = DIGBUFLEN ;
	int		rs ;
	int		len = 0 ;
	char		*bp = (rbuf + i) ;

	if (i < 0)
	    return SR_INVALID ;

	*bp = '\0' ;
	if ((rlen < 0) || ((rlen-i) >= dlen)) {
	    if ((rs = ctdeci(bp,(rlen-i),v)) >= 0) {
		len = rs ;
	    }
	} else {
	    char	dbuf[DIGBUFLEN + 1] ;
	    if ((rs = ctdeci(dbuf,dlen,v)) >= 0) {
	        len = rs ;
	        if ((rlen < 0) || ((rlen - i) >= len)) {
	            strwcpy(bp,dbuf,len) ;
	        } else
		    rs = SR_OVERFLOW ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (storebuf_deci) */


int storebuf_decui(char *rbuf,int rlen,int i,uint uv)
{
	const int	dlen = DIGBUFLEN ;
	int		rs ;
	int		len = 0 ;
	char		*bp = (rbuf + i) ;

	if (i < 0)
	    return SR_INVALID ;

	*bp = '\0' ;
	if ((rlen < 0) || ((rlen-i) >= dlen)) {
	    if ((rs = ctdecui(bp,(rlen-i),uv)) >= 0) {
		len = rs ;
	    }
	} else {
	    char	dbuf[DIGBUFLEN + 1] ;
	    if ((rs = ctdecui(dbuf,dlen,uv)) >= 0) {
	        len = rs ;
	        if ((rlen < 0) || ((rlen - i) >= len)) {
	            strwcpy(bp,dbuf,len) ;
	        } else
		    rs = SR_OVERFLOW ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (storebuf_decui) */


int storebuf_dec(char *rbuf,int rlen,int i,int v)
{
	return storebuf_deci(rbuf,rlen,i,v) ;
}
/* end subroutine (storebuf_dec) */


int storebuf_hexui(char *rbuf,int rlen,int i,uint uv)
{
	const int	dlen = HEXBUFLEN ;
	int		rs ;
	int		len = 0 ;
	char		*bp = (rbuf + i) ;

	if (i < 0)
	    return SR_INVALID ;

	*bp = '\0' ;
	if ((rlen < 0) || ((rlen-i) >= dlen)) {
	    if ((rs = cthexui(bp,(rlen-i),uv)) >= 0) {
		len = rs ;
	    }
	} else {
	    char	dbuf[HEXBUFLEN + 1] ;
	    if ((rs = cthexui(dbuf,dlen,uv)) >= 0) {
	        len = rs ;
	        if ((rlen < 0) || ((rlen - i) >= len)) {
	            strwcpy(bp,dbuf,len) ;
	        } else
		    rs = SR_OVERFLOW ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (storebuf_hexui) */


int storebuf_hexi(char *rbuf,int rlen,int i,int v)
{
	uint		uv = (uint) v ;
	return storebuf_hexui(rbuf,rlen,i,uv) ;
}
/* end subroutine (storebuf_hexi) */


