/* whitelist */

/* whitelist mail address management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_PARTIAL	0		/* partial-domain match */
#define	CF_SNWCPYOPAQUE	1		/* |snwcpyopaque(3dam)| */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages a mail whitelist (or blacklist).

	Notes:

	= Compile-time switch CF_SNWCPYOPAQUE

	I hope that turning this ON improves performance just a tad.


*******************************************************************************/


#define	WHITELIST_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<field.h>
#include	<sbuf.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"splitaddr.h"
#include	"whitelist.h"


/* local defines */

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX(MAILADDRLEN,2048)
#endif


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	snwcpyopaque(char *,int,cchar *,int) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* forward references */

int		whitelist_fileadd(WHITELIST *,cchar *) ;

static int	mkaddr(char *,int,cchar *,int) ;

#if	CF_PARTIAL
static int	cmpaddr() ;
#endif


/* local variables */

#if	CF_SNWCPYOPAQUE
#else /* CF_SNWCPYOPAQUE */
static const uchar	fterms[32] = {
	0x00, 0x04, 0x00, 0x00,
	0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;
#endif /* CF_SNWCPYOPAQUE */


/* exported subroutines */


int whitelist_open(WHITELIST *op,cchar *fname)
{
	const int	n = WHITELIST_DEFENTS ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	op->magic = 0 ;
	if ((rs = vecstr_start(&op->list,n,0)) >= 0) {
	    op->magic = WHITELIST_MAGIC ;
	    if (fname != NULL) {
	        rs = whitelist_fileadd(op,fname) ;
	    } /* end if */
	    if (rs < 0) {
	        vecstr_finish(&op->list) ;
	        op->magic = 0 ;
	    }
	}

	return rs ;
}
/* end subroutine (whitelist_open) */


int whitelist_fileadd(WHITELIST *op,cchar *fname)
{
	bfile		loadfile, *lfp = &loadfile ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (op->magic != WHITELIST_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("whitelist_fileadd: fname=%s\n",fname) ;
#endif

	if (fname[0] == '\0') return SR_INVALID ;

/* try to open it */

	if ((rs = bopen(lfp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(lfp,lbuf,llen)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;

	        if ((len > 0) && (lbuf[0] != '#')) {
	            const int	mlen = MAILADDRLEN ;
	            char	mbuf[MAILADDRLEN + 1] ;
	            if ((rs = mkaddr(mbuf,mlen,lbuf,len)) > 0) {
	                n += 1 ;
	                rs = vecstr_add(&op->list,mbuf,rs) ;
		    } else if (rs == SR_OVERFLOW) {
			rs = SR_OK ;
	            } /* end if */
	        } /* end if (positive) */

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    rs1 = bclose(lfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */

#if	CF_DEBUGS
	debugprintf("whitelist_fileadd: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (whitelist_fileadd) */


int whitelist_close(WHITELIST *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != WHITELIST_MAGIC) return SR_NOTOPEN ;

	rs1 = vecstr_finish(&op->list) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (whitelist_close) */


int whitelist_get(WHITELIST *op,int i,cchar **rpp)
{
	int		rs ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != WHITELIST_MAGIC) return SR_NOTOPEN ;

	rs = vecstr_get(&op->list,i,&cp) ;

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? cp : NULL ;

	return rs ;
}
/* end subroutine (whitelist_get) */


#ifdef	COMMENT

int whitelist_read(WHITELIST *op,int i,char *buf,int buflen)
{
	int		rs ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != WHITELIST_MAGIC) return SR_NOTOPEN ;

	rs = vecstr_get(&op->list,i,&cp) ;

	if (rs >= 0)
	    rs = sncpy1(buf,buflen,cp) ;

	return rs ;
}
/* end subroutine (whitelist_fetch) */

#endif /* COMMENT */


/* return the count of the number of items in this list */
int whitelist_count(WHITELIST *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != WHITELIST_MAGIC) return SR_NOTOPEN ;

	rs = vecstr_count(&op->list) ;

	return rs ;
}
/* end subroutine (whitelist_count) */


int whitelist_prematch(WHITELIST *op,cchar *ta)
{
	VECSTR		*lp = &op->list ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (ta == NULL) return SR_FAULT ;

	if (op->magic != WHITELIST_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("whitelist_prematch: test-addr=%s\n",ta) ;
#endif

	if (ta[0] == '\0') return SR_INVALID ;

#if	CF_PARTIAL

	if (strpbrk(ta,"@!") != NULL) {
	    if ((rs = vecstr_search(lp,ta,cmpaddr,NULL)) >= 0) {
	        f = TRUE ;
	    }
	}

#else /* CF_PARTIAL */
	{
	    SPLITADDR	ac ; /* address-candidate */
	    SPLITADDR	aw ; /* address-white */

	    if ((rs = splitaddr_start(&ac,ta)) >= 0) {
	        int	i ;
	        cchar	*cp ;

#if	CF_DEBUGS
	        debugprintf("whitelist_prematch: search for-before\n") ;
#endif

	        for (i = 0 ; vecstr_get(lp,i,&cp) >= 0 ; i += 1) {
	            if (cp != NULL) {
	                if ((rs = splitaddr_start(&aw,cp)) >= 0) {

	                    rs = splitaddr_prematch(&aw,&ac) ;
	                    f = (rs > 0) ;

#if	CF_DEBUGS
	                    debugprintf("whitelist_prematch: "
	                        "splitaddr_prematch() rs=%d\n",rs) ;
#endif

	                    rs1 = splitaddr_finish(&aw) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (splitaddr) */
	            } /* end if (non-null) */
	            if ((rs < 0) || f) break ;
	        } /* end for */

#if	CF_DEBUGS
	        debugprintf("whitelist_prematch: search for-after rs=%d\n",
	            rs) ;
#endif

	        rs1 = splitaddr_finish(&ac) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (splitaddr) */

	} /* end block */
#endif /* CF_PARTIAL */

#if	CF_DEBUGS
	debugprintf("whitelist_prematch: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (whitelist_prematch) */


#ifdef	COMMENT

int whitelist_curbegin(WHITELIST *op,WHITELIST_CUR *cp)
{

	if (op == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (whitelist_curbegin) */


int whitelist_curend(WHITELIST *op)
{

	if (op == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (whitelist_curend) */

#endif /* COMMENT */


int whitelist_audit(WHITELIST *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != WHITELIST_MAGIC) return SR_NOTOPEN ;

	rs = vecstr_audit(&op->list) ;

	return rs ;
}
/* end subroutine (whitelist_audit) */


/* private subroutines */


#if	CF_SNWCPYOPAQUE
static int mkaddr(char *mbuf,int mlen,cchar *lp,int ll)
{
	int		rs ;
	char		*tp ;
	if ((tp = strnchr(lp,ll,'#')) != NULL) {
	    ll = (tp-lp) ;
	}
	rs = snwcpyopaque(mbuf,mlen,lp,ll) ;
	return rs ;
}
/* end subroutine (mkaddr) */
#else /* CF_SNWCPYOPAQUE */
static int mkaddr(char *mbuf,int mlen,cchar *lp,int ll)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;
	if ((rs = sbuf_start(&b,mbuf,mlen)) >= 0) {
	    FIELD	fsb ;
	    if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	        int	fl ;
	        cchar	*fp ;
	        while ((fl = field_get(&fsb,fterms,&fp)) >= 0) {
	            if (fl > 0) {
	                rs = sbuf_strw(&b,fp,fl) ;
	            } /* end if (got one) */
	            if (fsb.term == '#') break ;
	            if (rs < 0) break ;
	        } /* end while */
	        field_finish(&fsb) ;
	    } /* end if (field) */
	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkaddr) */
#endif /* CF_SNWCPYOPAQUE */


#if	CF_PARTIAL

static int cmpaddr(cchar **e1pp,cchar **e2pp)
{
	int		rc ;
	cchar		*e1p, *e2p ;
	char		*h1p, *h2p ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	e1p = *e1pp ;
	e2p = *e2pp ;

	h1p = strchr(e1p,'@') ;

	h2p = strchr(e2p,'@') ;

	if ((h1p == NULL) || (h2p == NULL)) {
	    if (h1p == NULL) {
	        rc = strcasecmp(e1p,(h2p + 1)) ;
	    } else {
	        rc = strcasecmp((h1p + 1),e2p) ;
	    }
	} else
	    rc = strcasecmp(e1p,e2p) ;

	return rc ;
}
/* end subroutine (cmpaddr) */

#endif /* CF_PARTIAL */


