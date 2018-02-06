/* storeitem */

/* storage object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-02-17, David A­D­ Morano
        This object was originally written for use in some old PCS code that is
        being reworked.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module can be used to construct strings or messages in a buffer
        WITHOUT using the 'sprintf(3c)' subroutine.

	Arguments:

	op		pointer to the storage object
	<others>	depending on the particular call made

	Returns:

	>=0		length of the newly stored item
	<0		error


*******************************************************************************/


#define	STOREITEM_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"storeitem.h"


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern ulong	ulceil(ulong,int) ;

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	ctdeci(char *,int,int) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int storeitem_start(STOREITEM *op,char dbuf[],int dlen)
{

#if	CF_DEBUGS
	debugprintf("storeitem_start: ent dlen=%d\n",dlen) ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (dbuf == NULL) return SR_FAULT ;

	if (dlen < 0)
	    return SR_INVALID ;

	op->dbuf = dbuf ;
	op->dlen = dlen ;
	op->index = 0 ;
	op->f_overflow = FALSE ;
	return SR_OK ;
}
/* end subroutine (storeitem_start) */


int storeitem_finish(STOREITEM *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->dbuf == NULL) return SR_NOTOPEN ;

	if (op->f_overflow) rs = SR_OVERFLOW ;

	if (rs >= 0)
	    rs = (op->index + 1) ;

	op->dbuf = NULL ;
	op->index = -1 ;
	return rs ;
}
/* end subroutine (storeitem_finish) */


int storeitem_strw(STOREITEM *op,cchar sp[],int sl,cchar **rpp)
{
	int		rs = SR_OK ;
	int		dlen ;
	char		*dbuf, *dp ;

	if (op == NULL) return SR_FAULT ;

	if (op->dbuf == NULL) return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = NULL ;

	if (op->index < 0)
	    return op->index ;

	dbuf = (op->dbuf + op->index) ;
	dlen = (op->dlen - op->index) ;

	dp = dbuf ;
	if (rpp != NULL)
	    *rpp = dbuf ;

	while (sl-- && *sp && dlen--) {
	    *dp++ = *sp++ ;
	}

	if (dlen < 0) {
	    op->f_overflow = TRUE ;
	    rs = SR_OVERFLOW ;
	}

	*dp = '\0' ;
	if (rs >= 0)
	    op->index += (dp - dbuf + 1) ;

	return (rs >= 0) ? (dp - dbuf) : rs ;
}
/* end subroutine (storeitem_strw) */


/* store a buffer into the item-buffer */
int storeitem_buf(STOREITEM *op,const void *vbp,int vbl,cchar **rpp)
{
	int		rs = SR_OK ;
	int		sl = vbl ;
	int		dlen ;
	const char	*sp = (const char *) vbp ;
	char		*dbuf, *dp ;

	if (op == NULL) return SR_FAULT ;

	if (op->dbuf == NULL) return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = NULL ;

	if (op->index < 0)
	    return op->index ;

	dbuf = (op->dbuf + op->index) ;
	dlen = (op->dlen - op->index) ;

	dp = dbuf ;
	if (rpp != NULL)
	    *rpp = dbuf ;

	while (sl-- && dlen--) {
	    *dp++ = *sp++ ;
	}

	if (dlen < 0) {
	    op->f_overflow = TRUE ;
	    rs = SR_OVERFLOW ;
	}

	*dp = '\0' ;
	if (rs >= 0) {
	    op->index += (dp - dbuf + 1) ;
	}

	return (rs >= 0) ? (dp - dbuf) : rs ;
}
/* end subroutine (storeitem_buf) */


/* store a decimal number (as a string) into the buffer */
int storeitem_dec(STOREITEM *op,int v,cchar **rpp)
{
	int		rs = SR_OK ;
	int		dlen ;
	int		len = 0 ;
	char		*dbuf ;

	if (op == NULL) return SR_FAULT ;

	if (op->dbuf == NULL) return SR_NOTOPEN ;

	if (op->index < 0)
	    return op->index ;

	dbuf = (op->dbuf + op->index) ;
	dlen = (op->dlen - op->index) ;

	if (op->dlen >= 0) {
	    const int	tlen = DIGBUFLEN ;
	    if (dlen >= (tlen+1)) {
	        rs = ctdeci(dbuf,tlen,v) ;
	        len = rs ;
	    } else {
	        char	tbuf[DIGBUFLEN+1] ;
	        if ((rs = ctdeci(tbuf,tlen,v)) >= 0) {
		    if (dlen >= (rs+1)) {
			rs = snwcpy(dbuf,dlen,tbuf,rs) ;
			len = rs ;
		    } else {
	                op->f_overflow = TRUE ;
	                rs = SR_OVERFLOW ;
		    }
		}
	    }
	} /* end if (possible) */

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? dbuf : NULL ;
	}

	if (rs >= 0) {
	    op->index += (len + 1) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (storeitem_dec) */


int storeitem_char(STOREITEM *op,int ch,cchar **rpp)
{
	const int	len = 1 ;
	int		rs = SR_OK ;
	int		dlen ;
	char		*dbuf ;

	if (op == NULL) return SR_FAULT ;

	if (op->dbuf == NULL) return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = NULL ;

	if (op->index < 0)
	    return op->index ;

	dbuf = (op->dbuf + op->index) ;
	dlen = (op->dlen - op->index) ;

	if ((op->dlen >= 0) && ((len+1) <= dlen)) {
	    char	*bp = dbuf ;
	    *bp++ = ch ;
	    *bp = '\0' ;
	} else {
	    op->f_overflow = TRUE ;
	    rs = SR_OVERFLOW ;
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? dbuf : NULL ;
	}

	if (rs >= 0) {
	    op->index += (len + 1) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (storeitem_char) */


int storeitem_nul(STOREITEM *op,cchar **rpp)
{

	return storeitem_strw(op,op->dbuf,0,rpp) ;
}
/* end subroutine (storeitem_nul) */


/* store a table of pointers */
int storeitem_ptab(STOREITEM *op,int n,void ***vppp)
{
	ulong		v, nv ;
	const int	isize = sizeof(void *) ;
	int		rs = SR_OK ;
	int		inc = 0 ;
	char		*bp ;

	if (op == NULL) return SR_FAULT ;
	if (vppp == NULL) return SR_FAULT ;

	if (n < 0) return SR_INVALID ;

	if (op->index < 0) return op->index ;

	bp = (op->dbuf + op->index) ;
	v = (ulong) bp ;
	nv = ulceil(v,isize) ;

	if (vppp != NULL)
	    *vppp = (void **) nv ;

	nv += ((n + 1) * isize) ;

	inc = (nv - v) ;
	if ((op->dlen < 0) || (inc <= op->dlen)) {
	    memset(bp,0,inc) ;
	    if (op->dlen >= 0) op->index += inc ;
	} else {
	    rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? inc : rs ;
}
/* end subroutine (storeitem_ptab) */


/* store am array of memory-aligned blocks */
int storeitem_block(STOREITEM *op,int bsize,int align,void **vpp)
{
	ulong		v, nv ;
	int		rs = SR_OK ;
	int		inc = 0 ;
	char		*bp ;

	if (op == NULL) return SR_FAULT ;
	if (vpp == NULL) return SR_FAULT ;

	if (bsize < 1) return SR_INVALID ;
	if (align < 1) return SR_INVALID ;

	if (op->index < 0) return op->index ;

	bp = (op->dbuf + op->index) ;
	v = (ulong) bp ;
	nv = ulceil(v,align) ;

	if (vpp != NULL)
	    *vpp = (void *) nv ;

	nv += bsize ;

	inc = (nv - v) ;
	if ((op->dlen < 0) || (inc <= op->dlen)) {
	    memset(bp,0,inc) ;
	    if (op->dlen >= 0) op->index += inc ;
	} else {
	    rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? inc : rs ;
}
/* end subroutine (storeitem_block) */


int storeitem_getlen(STOREITEM *op)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->dbuf == NULL) return SR_NOTOPEN ;

	if (op->f_overflow) rs = SR_OVERFLOW ;

	len = op->index ;
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (storeitem_getlen) */


