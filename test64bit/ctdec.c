/* ctdec */

/* subroutines to convert an integer to a decimal string */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

        These subroutines convert an integer (signed or unsigned) into a decimal
        string that is placed into the caller supplied buffer (of specified
        length).

	Synopsis:

	int ctdecXX(rbuf,rlen,v)
	char		rbuf[] ;
	int		rlen ;
	int		v ;

	Arguments:

	rbuf		caller supplied buffer
	rlen		caller supplied buffer length
	v		integer value to be converted

	Returns:

	>=0		length of buffer used by the conversion
	<0		error in the conversion

	Notes:

        As it stands now, these subroutines do not perform any funny business in
        trying to make this process faster! These subroutines are, therefore,
        probably the slowest such conversions routinely available. To really
        move (execute) quickly through the division-related aspects of the
        require algorithm, one would have to use assembly language where both
        the quotient and the reminder of a division are produced simultaneously
        (since each are needed to continue). This, of course, assumes that the
        underlying machine architecture has such instructions. But short of
        assembly (and and the required machine instructions) this present
        implemtnation is adequate.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define MAXDECDIG_I	10		/* decimal digits in 'int' */
#define MAXDECDIG_UI	10		/* decimal digits in 'uint' */
#define MAXDECDIG_L	10		/* decimal digits in 'long' */
#define MAXDECDIG_UL	10		/* decimal digits in 'ulong' */
#define MAXDECDIG_L64	19		/* decimal digits in 'long64' */
#define MAXDECDIG_UL64	20		/* decimal digits in 'ulong64' */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexi(const char *,int) ;
#endif


/* forward references */

int		ctdeci(char *,int,int) ;

static int	ictdec(char *,int,ulonglong) ;


/* local variables */


/* exported subroutines */


int ctdec(char *dbuf,int dlen,int v)
{

	return ctdeci(dbuf,dlen,v) ;
}
/* end subroutine (ctdec) */


int ctdeci(char *dbuf,int dlen,int v)
{
	ulonglong	ulv = (ulonglong) v ;
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

	if (v < 0) ulv = (- ulv) ;
	len = ictdec(digbuf,diglen,ulv) ;
	if (v < 0) digbuf[diglen-(++len)] = '-' ;

	rs = sncpy1(dbuf,dlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctdeci) */


/* convert to an unsigned integer */
int ctdecui(char *dbuf,int dlen,uint v)
{
	ulonglong	ulv = (ulonglong) v ;
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

	len = ictdec(digbuf,diglen,ulv) ;

	rs = sncpy1(dbuf,dlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctdecui) */


/* convert to a signed long integer */
int ctdecl(char *dbuf,int dlen,long v)
{
	ulonglong	ulv = (ulonglong) v ;
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

	if (v < 0) ulv = (- ulv) ;
	len = ictdec(digbuf,diglen,ulv) ;
	if (v < 0) digbuf[diglen-(++len)] = '-' ;

	rs = sncpy1(dbuf,dlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctdecl) */


/* convert to an unsigned long integer */
int ctdecul(char *dbuf,int dlen,ulong v)
{
	ulonglong	ulv = (ulonglong) v ;
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

	len = ictdec(digbuf,diglen,ulv) ;

	rs = sncpy1(dbuf,dlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctdecul) */


/* convert to a signed long-long integer */
int ctdecll(char *dbuf,int dlen,longlong v)
{
	ulonglong	ulv = (ulonglong) v ;
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

	if (v < 0) ulv = (- ulv) ;
	len = ictdec(digbuf,diglen,ulv) ;
	if (v < 0) digbuf[diglen-(++len)] = '-' ;

	rs = sncpy1(dbuf,dlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctdecll) */


/* convert to an unsigned long-long integer */
int ctdecull(char *dbuf,int dlen,ulonglong v)
{
	ulonglong	ulv = (ulonglong) v ;
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

	len = ictdec(digbuf,diglen,ulv) ;

	rs = sncpy1(dbuf,dlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctdecull) */


/* local subroutines */


static int ictdec(char *dbuf,int dlen,ulonglong v)
{
	const int	b = 10 ;
	char		*rp = (dbuf + dlen) ;

	*rp = '\0' ;
	if (v != 0) {

#if	defined(_ILP32) /* may speed things up on some machines */
	    if ((v & (~ULONG_MAX)) == 0LL) {
		ulong	lv = (ulong) v ;
		ulong	nv ;
		while (lv != 0) {
	            nv = lv / b ;
	            *--rp = (char) ((lv - (nv * b)) + '0') ;
	            lv = nv ;
		} /* end while */
		v = lv ;
	    }
#endif /* _ILP32 */
	    {
		ulonglong	nv ;
	        while (v != 0) {
	            nv = v / b ;
	            *--rp = (char) ((v - (nv * b)) + '0') ;
	            v = nv ;
	        } /* end while */
	    }

	} else
	    *--rp = '0' ;

#if	CF_DEBUGS
	{
	    int len = (dbuf + dlen - rp) ;
	    debugprintf("ictdec: ret len=%u\n",len) ;
	    debugprintf("ictdec: ret b=>%s<\n",rp) ;
	}
#endif

	return (dbuf + dlen - rp) ;
}
/* end subroutine (ictdec) */


