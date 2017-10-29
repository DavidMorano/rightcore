/* sbuf */

/* storage buffer (SBuf) object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano

	This object module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module can be used to construct strings or messages in
	buffers WITHOUT using the 'sprint(3c)' subroutine or something
	similar.

	This module is useful when the user SUPPLIES a buffer of a
	specified length to the object at object initialization.

	This module uses an object, that must be initialized and
	eventually freed, to track the state of the user supplied buffer.

	Arguments:

	bop		pointer to the buffer object
	<others>	depending on the call made

	Returns:

	>=0		amount of new space used by the newly stored item
			(not including any possible trailing NUL characters)
	<0		error


*******************************************************************************/


#define	SBUF_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<format.h>
#include	<localmisc.h>

#include	"sbuf.h"


/* local defines */

#define	SBUF_DBUF	(sbp->dbuf)
#define	SBUF_DLEN	(sbp->dlen)
#define	SBUF_INDEX	(sbp->index)

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	45		/* can hold int128_t in decimal */
#endif

#ifndef	HEXBUFLEN
#define	HEXBUFLEN	32		/* can hold int128_t in hexadecimal */
#endif


/* external subroutines */

extern int	ctdeci(char *,int,int) ;
extern int	ctdecl(char *,int,long) ;
extern int	ctdecll(char *,int,longlong) ;
extern int	ctdecui(char *,int,uint) ;
extern int	ctdecul(char *,int,ulong) ;
extern int	ctdecull(char *,int,ulonglong) ;
extern int	cthexi(char *,int) ;
extern int	cthexl(char *,long) ;
extern int	cthexll(char *,longlong) ;
extern int	cthexui(char *,uint) ;
extern int	cthexul(char *,ulong) ;
extern int	cthexull(char *,ulonglong) ;


/* local structures */


/* forward references */

int		sbuf_buf(SBUF *,const char *,int) ;
int		sbuf_vprintf(SBUF *,const char *,va_list) ;

static int	sbuf_addstrw(SBUF *,const char *,int) ;

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,void *,int) ;
#endif


/* local variables */

static const char	*blanks = "        " ;


/* exported subroutines */


int sbuf_start(SBUF *sbp,char *dbuf,int dlen)
{

#if	CF_DEBUGS
	debugprintf("sbuf_start: dlen=%d\n",dlen) ;
#endif

	if (sbp == NULL) return SR_FAULT ;
	if (dbuf == NULL) return SR_FAULT ;

	dbuf[0] = '\0' ;
	SBUF_DBUF = dbuf ;
	SBUF_DLEN = dlen ;
	SBUF_INDEX = 0 ;
	return SR_OK ;
}
/* end subroutine (sbuf_start) */


int sbuf_finish(SBUF *sbp)
{
	int		rs ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	rs = SBUF_INDEX ;

#if	CF_DEBUGS
	debugprintf("sbuf_finish: rs=%d\n",rs) ;
#endif

	SBUF_DBUF = NULL ;
	SBUF_DLEN = 0 ;
	SBUF_INDEX = SR_NOTOPEN ;
	return rs ;
}
/* end subroutine (sbuf_finish) */


/* store a memory buffer in the object buffer */
int sbuf_buf(SBUF *sbp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		len ;
	char		*bp ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	bp = (SBUF_DBUF + SBUF_INDEX) ;
	if (SBUF_DLEN < 0) {

	    if (sl < 0) {
	        while (*sp) {
	            *bp++ = *sp++ ;
		}
	    } else {
	        while (sl > 0) {
	            *bp++ = *sp++ ;
	            sl -= 1 ;
	        }
	    } /* end if */

	} else {

	    if (sl < 0) {
	        while (*sp && (bp < (SBUF_DBUF + SBUF_DLEN))) {
	            *bp++ = *sp++ ;
		}
	    } else {
	        while ((sl > 0) && (bp < (SBUF_DBUF + SBUF_DLEN))) {
	            *bp++ = *sp++ ;
	            sl -= 1 ;
	        }
	    } /* end if */
	    if (*sp && (sl > 0)) rs = SR_OVERFLOW ;

	} /* end if */

	*bp = '\0' ;
	len = bp - (SBUF_DBUF + SBUF_INDEX) ;
	SBUF_INDEX = (rs >= 0) ? (bp - SBUF_DBUF) : rs ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_buf) */


/* store a string into the buffer and return a pointer to it */
int sbuf_strw(SBUF *sbp,cchar *sp,int sl)
{

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	return sbuf_addstrw(sbp,sp,sl) ;
}
/* end subroutine (sbuf_strw) */


/* store a decimal number (as a string) into the buffer */
int sbuf_deci(SBUF *sbp,int v)
{
	int		rs ;
	int		len = 0 ;
	char		digbuf[DIGBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = ctdeci(digbuf,DIGBUFLEN,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,digbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_deci) */


int sbuf_decl(SBUF *sbp,long v)
{
	int		rs ;
	int		len = 0 ;
	char		digbuf[DIGBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = ctdecl(digbuf,DIGBUFLEN,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,digbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_decl) */


int sbuf_decll(SBUF *sbp,longlong v)
{
	int		rs ;
	int		len = 0 ;
	char		digbuf[DIGBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = ctdecll(digbuf,DIGBUFLEN,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,digbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_decll) */


/* store an unsigned  decimal number (as a string) into the buffer */
int sbuf_decui(SBUF *sbp,uint v)
{
	int		rs ;
	int		len = 0 ;
	char		digbuf[DIGBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = ctdecui(digbuf,DIGBUFLEN,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,digbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_decui) */


int sbuf_decul(SBUF *sbp,ulong v)
{
	int		rs ;
	int		len = 0 ;
	char		digbuf[DIGBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = ctdecul(digbuf,DIGBUFLEN,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,digbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_decul) */


int sbuf_decull(SBUF *sbp,ulonglong v)
{
	int		rs ;
	int		len = 0 ;
	char		digbuf[DIGBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = ctdecull(digbuf,DIGBUFLEN,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,digbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_decull) */


/* store a hexadecimal number (as a string) into the buffer */
int sbuf_hexi(SBUF *sbp,int v)
{
	int		rs ;
	int		len = 0 ;
	char		hexbuf[HEXBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = cthexi(hexbuf,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,hexbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_hexi) */


int sbuf_hexl(SBUF *sbp,long v)
{
	int		rs ;
	int		len = 0 ;
	char		hexbuf[HEXBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = cthexl(hexbuf,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,hexbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_hexl) */


int sbuf_hexll(SBUF *sbp,longlong v)
{
	int		rs ;
	int		len = 0 ;
	char		hexbuf[HEXBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = cthexll(hexbuf,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,hexbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_hexll) */


int sbuf_hex(SBUF *sbp,int v)
{

	return sbuf_hexi(sbp,v) ;
}
/* end subroutine (sbuf_hex) */


int sbuf_hexui(SBUF *sbp,uint v)
{
	int		rs ;
	int		len = 0 ;
	char		hexbuf[HEXBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = cthexui(hexbuf,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,hexbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_hexui) */


int sbuf_hexul(SBUF *sbp,ulong v)
{
	int		rs ;
	int		len = 0 ;
	char		hexbuf[HEXBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = cthexul(hexbuf,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,hexbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_hexul) */


int sbuf_hexull(SBUF *sbp,ulonglong v)
{
	int		rs ;
	int		len = 0 ;
	char		hexbuf[HEXBUFLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	if ((rs = cthexull(hexbuf,v)) >= 0) {
	    len = rs ;
	    rs = sbuf_addstrw(sbp,hexbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_hexull) */


/* store a character */
int sbuf_char(SBUF *sbp,int ch)
{
	int		rs = SR_OK ;
	int		ni ;
	int		len = 1 ;
	char		*bp ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	bp = (SBUF_DBUF + SBUF_INDEX) ;
	ni = (SBUF_INDEX + len) ;
	if (SBUF_DLEN >= 0) {
	    if (SBUF_DLEN >= ni) {
		*bp++ = ch ;
	    } else
		rs = SR_OVERFLOW ;
	} else
	    *bp++ = ch ;

	if (rs >= 0) {
	    SBUF_INDEX = ni ;
	    *bp = '\0' ;
	} else
	    SBUF_INDEX = rs ;

#if	CF_DEBUGS
	debugprintf("sbuf_char: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_char) */


int sbuf_blanks(SBUF *sbp,int n)
{
	const int	nblank = sizeof(blanks) ;
	int		rs = SR_OK ;
	int		m ;
	int		len = 0 ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX < 0) return SBUF_INDEX ;

	while ((rs >= 0) && (len < n)) {
	    m = MIN(nblank,(n-len)) ;
	    rs = sbuf_addstrw(sbp,blanks,m) ;
	    len += m ;
	} /* end while */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_blanks) */


/* vprintf-like thing */
int sbuf_vprintf(SBUF *sbp,cchar *fmt,va_list ap)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (sbp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (SBUF_INDEX >= 0) {
	    const int	fm = 0x01 ; /* *will* error out on overflow! */
	    int		dl = (SBUF_DLEN - SBUF_INDEX) ;
	    char	*dp = (SBUF_DBUF + SBUF_INDEX) ;
	    if ((rs = format(dp,dl,fm,fmt,ap)) >= 0) {
	        len = rs ;
		SBUF_INDEX += len ;
	    } else
		SBUF_INDEX = rs ;
	} else
	    rs = SBUF_INDEX ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_vprintf) */


/* PRINTFLIKE2 */
int sbuf_printf(SBUF *op,const char *fmt,...)
{
	int		rs ;
	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = sbuf_vprintf(op,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (sbuf_printf) */


int sbuf_adv(SBUF *sbp,int adv,char **dpp)
{
	int		rs = SR_OK ;

	if (sbp == NULL) return SR_FAULT ;

	if (dpp != NULL) *dpp = NULL ;
	if (SBUF_INDEX >= 0) {
	    if ((SBUF_DLEN - SBUF_INDEX) >= adv) {
		if (dpp != NULL) *dpp = (SBUF_DBUF + SBUF_INDEX) ;
		SBUF_INDEX += adv ;
	    } else
		rs = SR_TOOBIG ;
	} else
	    rs = SBUF_INDEX ;

	return (rs >= 0) ? adv : rs ;
}
/* end subroutine (sbuf_adv) */


int sbuf_rem(SBUF *sbp)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX >= 0) {
	    rs = (sbp->dlen - sbp->index) ;
	} else
	    rs = SBUF_INDEX ;

	return rs ;
}
/* end subroutine (sbuf_rem) */


/* get the length filled so far */
int sbuf_getlen(SBUF *sbp)
{

	if (sbp == NULL) return SR_FAULT ;

	return SBUF_INDEX ;
}
/* end subroutine (sbuf_getlen) */


/* get the pointer in the buffer to the next character */
int sbuf_getpoint(SBUF *sbp,cchar **rpp)
{

	if (sbp == NULL) return SR_FAULT ;

	if (rpp != NULL)
	    *rpp = (SBUF_DBUF + SBUF_INDEX) ;

	return SBUF_INDEX ;
}
/* end subroutine (sbuf_getpoint) */


/* get the previous character (if there is one) */
int sbuf_getprev(SBUF *sbp)
{
	int		rs = SR_NOENT ;

	if (sbp == NULL) return SR_FAULT ;

	if (SBUF_INDEX > 0)
	    rs = MKCHAR(SBUF_DBUF[SBUF_INDEX - 1]) ;

	return rs ;
}
/* end subroutine (sbuf_getprev) */


int sbuf_dec(SBUF *sbp,int v)
{

	return sbuf_deci(sbp,v) ;
}
/* end subroutine (sbuf_dec) */


/* private subroutines */


static int sbuf_addstrw(SBUF *sbp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		len ;
	char		*bp ;

	bp = (SBUF_DBUF + SBUF_INDEX) ;
	if (SBUF_DLEN < 0) {

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
	        while (*sp && (bp < (SBUF_DBUF + SBUF_DLEN))) {
	            *bp++ = *sp++ ;
		}
	        if (*sp) rs = SR_OVERFLOW ;
	    } else {
	        while (sl && *sp && (bp < (SBUF_DBUF + SBUF_DLEN))) {
	            *bp++ = *sp++ ;
	            sl -= 1 ;
	        }
	        if (*sp && (sl > 0)) rs = SR_OVERFLOW ;
	    } /* end if */

	} /* end if */

	*bp = '\0' ;
	len = bp - (SBUF_DBUF + SBUF_INDEX) ;
	SBUF_INDEX = (rs >= 0) ? (bp - SBUF_DBUF) : rs ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_addstrw) */


