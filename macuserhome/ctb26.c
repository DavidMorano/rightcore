/* ctb26 */

/* subroutines to convert an integer to a base-26 string */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_REMAINDER	0		/* use remainder-division */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines convert an integer (signed or unsigned) into a base-26
        string that is placed into the caller supplied buffer (of specified
        length).

	Synopsis:
	int ctb26XX(rbuf,rlen,type,prec,v)
	char		rbuf[] ;
	int		rlen ;
	int		type ;
	int		prec ;
	int		v ;

	Arguments:
	buf		caller supplied buffer
	buflen		caller supplied buffer length
	type		which alphabet 'a' or 'A'
	prec		minimum precision
	v		integer value to be converted

	Returns:
	>=0		length of buffer used by the conversion
	<0		error in the conversion


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

#define	DIGBASE		26		/* digit base */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexi(const char *,int) ;
#endif


/* forward references */

int		ctb26i(char *,int,int,int,int) ;

static int	ictb26(char *,int,int,int,ULONG) ;


/* local variables */


/* exported subroutines */


int ctb26(char *rbuf,int rlen,int type,int prec,int v)
{

	return ctb26i(rbuf,rlen,type,prec,v) ;
}
/* end subroutine (ctb26) */


int ctb26i(char *rbuf,int rlen,int type,int prec,int v)
{
	ULONG		ulv = (ULONG) v ;
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprinthexi("ctb26i: ent prec=",prec) ;
#endif

	if (v < 0) ulv = (- ulv) ;
	len = ictb26(digbuf,diglen,type,prec,ulv) ;
	if (v < 0) digbuf[diglen-(++len)] = '-' ;

	rs = sncpy1(rbuf,rlen,(digbuf + diglen - len)) ;

#if	CF_DEBUGS
	debugprinthexi("ctb26i: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ctb26i) */


/* convert to an unsigned integer */
int ctb26ui(char *rbuf,int rlen,int type,int prec,uint v)
{
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("ctb26ui: ent\n") ;
#endif

	len = ictb26(digbuf,diglen,type,prec,(ULONG) v) ;

#if	CF_DEBUGS
	debugprintf("ctb26ui: res=%s\n",
	    (digbuf + diglen - len)) ;
#endif

	rs = sncpy1(rbuf,rlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctb26ui) */


/* convert to a signed long integer */
int ctb26l(char *rbuf,int rlen,int type,int prec,long v)
{
	ULONG		ulv = (ULONG) v ;
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("ctb26l: ent\n") ;
#endif

	if (v < 0) ulv = (- ulv) ;
	len = ictb26(digbuf,diglen,type,prec,ulv) ;
	if (v < 0) digbuf[diglen-(++len)] = '-' ;

	rs = sncpy1(rbuf,rlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctb26l) */


/* convert to an unsigned long integer */
int ctb26ul(char *rbuf,int rlen,int type,int prec,ulong v)
{
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("ctb26ul: ent\n") ;
#endif

	len = ictb26(digbuf,diglen,type,prec,(ULONG) v) ;

#if	CF_DEBUGS
	debugprintf("ctb26ul: res=%s\n",
	    (digbuf + diglen - len)) ;
#endif

	rs = sncpy1(rbuf,rlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctb26ul) */


/* convert to a signed long-long integer */
int ctb26ll(char *rbuf,int rlen,int type,int prec,LONG v)
{
	ULONG		ulv = (ULONG) v ;
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("ctb26ll: ent\n") ;
#endif

	if (v < 0) ulv = (- ulv) ;
	len = ictb26(digbuf,diglen,type,prec,ulv) ;
	if (v < 0) digbuf[diglen-(++len)] = '-' ;

	rs = sncpy1(rbuf,rlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctb26ll) */


/* convert to an unsigned long-long integer */
int ctb26ull(char *rbuf,int rlen,int type,int prec,ULONG v)
{
	const int	diglen = DIGBUFLEN ;
	int		rs ;
	int		len ;
	char		digbuf[DIGBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("ctb26ull: ent\n") ;
#endif

	len = ictb26(digbuf,diglen,type,prec,(ULONG) v) ;

#if	CF_DEBUGS
	debugprintf("ctb26ull: res=%s\n",
	    (digbuf + diglen - len)) ;
#endif

	rs = sncpy1(rbuf,rlen,(digbuf + diglen - len)) ;

	return rs ;
}
/* end subroutine (ctb26ull) */


/* local subroutines */


static int ictb26(char *rbuf,int rlen,int type,int prec,ULONG v)
{
	const int	base = DIGBASE ;
	char		*rp ;

#if	CF_DEBUGS
	debugprintf("ictb26: ent\n") ;
#endif

	rp = rbuf + rlen ;
	*rp = '\0' ;
	if (v != 0) {
	    while (v != 0) {
#if	CF_REMAINDER
	        *--rp = (char) ((v % base) + type) ;
	        v = (v / base) ;
#else
		{
		    ULONG	nv ;
	            nv = v / base ;
	            *--rp = (char) ((v - (nv * base)) + type) ;
	            v = nv ;
		}
#endif /* CF_REMAINDER */
	    } /* end while */
	} else
	    *--rp = type ;

	{
		int	n = ((rbuf + rlen) - rp) ;
		while (n++ < prec) *--rp = type ;
	}

#if	CF_DEBUGS
	{
	int len = (rbuf + rlen - rp) ;
	debugprinthexi("ictb26: ret len=",len) ;
	debugprintf("ictb26: ret res=>%s<\n",rp) ;
	}
#endif

	return (rbuf + rlen - rp) ;
}
/* end subroutine (ictb26) */


