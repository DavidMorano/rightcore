/* chartrans */
/* lang=C99 */

/* character translation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */	


/* revision history:

	= 2014-07-15, David A­D­ Morano
	This is a new attempt at character translation handling.

*/

/* Copyright © 2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We deal with translating from some given character sets to the
	wide-character ('wchar_t') format.


*******************************************************************************/


#define	CHARTRANS_MASTER	0	/* do not claim */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<stddef.h>		/* presumably for type 'wchar_t' */
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<utf8decoder.h>
#include	<localmisc.h>

#include	"chartrans.h"


/* local defines */

#ifndef	CSNLEN
#define	CSNLEN		MAXNAMELEN
#endif

#ifdef	_BIG_ENDIAN
#define	CHARTRANS_SUF	"BE"
#else
#define	CHARTRANS_SUF	"LE"
#endif


/* external subroutines */

extern int	wsnwcpynarrow(wchar_t *,int,cchar *,int) ;
extern int	matcasestr(cchar **,cchar *,int) ;
extern int	strwcmp(cchar *,cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strnwcpy(char *,int,cchar *,int) ;
extern char	*strwcpyuc(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;


/* local structures */


/* forward references */

static int chartrans_setfins(CHARTRANS *) ;
static int chartrans_sethave(CHARTRANS *,cchar *,int) ;
static int chartrans_setopen(CHARTRANS *,time_t,int,cchar *,int) ;
static int chartrans_setclose(CHARTRANS *,int) ;
static int chartrans_setfind(CHARTRANS *,time_t) ;
static int chartrans_transutf8(CHARTRANS *,wchar_t *,int,cchar *,int) ;

static int mktransname(char *,int,cchar *,int) ;


/* local variables */

static cchar	*charsets[] = {
	"utf-8",
	"iso-8859-1",
	"iso-Latin-1",
	"Latin-1",
	"us-ascii",
	"ascii",
	NULL
} ;

enum charsets {
	charset_utf8,
	charset_latin1a,
	charset_latin1b,
	charset_latin1c,
	charset_asciia,
	charset_asciib,
	charset_overlast
} ;


/* exported subroutines */


int chartrans_open(CHARTRANS *op ,cchar *pr,int maxtx)
{
	int		rs ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	if (maxtx < 1) maxtx = 1 ;

	memset(op,0,sizeof(CHARTRANS)) ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    const int	asize = (maxtx * sizeof(CHARTRANS_SET)) ;
	    void	*p ;
	    op->pr = cp ;
	    if ((rs = uc_malloc(asize,&p)) >= 0) {
	        op->sets = (CHARTRANS_SET *) p ;
	        op->nmax = maxtx ;
	        op->magic = CHARTRANS_MAGIC ;
	        memset(p,0,asize) ;
	    } /* end if (memory-allocation) */
	    if (rs < 0) {
	        uc_free(op->pr) ;
	        op->pr = NULL ;
	    }
	} /* end if (memory_allocation) */

	return rs ;
}
/* end subroutine (chartrans_open) */


int chartrans_close(CHARTRANS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->utf8decoder != NULL) {
	    UTF8DECODER	*uop = (UTF8DECODER *) op->utf8decoder ;
	    rs1 = utf8decoder_finish(uop) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(op->utf8decoder) ;
	    if (rs >= 0) rs = rs1 ;
	    op->utf8decoder = NULL ;
	}

	if (op->nsets > 0) {
	    rs1 = chartrans_setfins(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->sets != NULL) {
	    rs1 = uc_free(op->sets) ;
	    if (rs >= 0) rs = rs1 ;
	    op->sets = NULL ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (chartrans_close) */


int chartrans_transbegin(CHARTRANS *op,time_t dt,cchar *np,int nl)
{
	int		rs ;
	int		txid = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

	if (nl < 0) nl = strlen(np) ;

#if	CF_DEBUGS
	debugprintf("chartrans_transbegin: ent n=%t\n",np,nl) ;
#endif

	if ((rs = chartrans_sethave(op,np,nl)) >= 0) {
	    txid = rs ;
	    op->sets[txid].uc += 1 ;
	    op->sets[txid].acount = op->acount++ ;
	} else if (rs == SR_NOTFOUND) {
	    if ((rs = chartrans_setfind(op,dt)) >= 0) {
	        CHARTRANS_SET	*setp = (op->sets + rs) ;
	        txid = rs ;
	        if (setp->name != NULL) {
	            rs = chartrans_setclose(op,txid) ;
	        }
	        if (rs >= 0) {
	            if ((rs = chartrans_setopen(op,dt,txid,np,nl)) >= 0) {
	                op->sets[txid].uc += 1 ;
	            }
	        }
	    } /* end if (chartrans_setfind) */
	} /* end if (have it) */

#if	CF_DEBUGS
	debugprintf("chartrans_transbegin: ret rs=%d txid=%u\n",rs,txid) ;
#endif

	return (rs >= 0) ? txid : rs ;
}
/* end subroutine (chartrans_transbegin) */


int chartrans_transend(CHARTRANS *op,int txid)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if ((txid < 0) || (txid >= op->nmax)) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("chartrans_transend: ent txid=%u\n",txid) ;
#endif

	if (rs >= 0) {
	    CHARTRANS_SET	*setp = (op->sets + txid) ;
	    if (setp->uc > 0) setp->uc -= 1 ;
	}

#if	CF_DEBUGS
	debugprintf("chartrans_transbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (chartrans_transend) */


int chartrans_transread(CHARTRANS *op,int txid,wchar_t *rcp,int rcl,
		cchar *sp,int sl)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (rcp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (rcl < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("chartrans_transread: txid=%u\n",txid) ;
#endif

	if ((txid < 0) || (txid >= op->nmax)) return SR_INVALID ;

	if (sl < 0) sl = strlen(sp) ;

	if (rs >= 0) {
	    CHARTRANS_SET	*setp = (op->sets + txid) ;
#if	CF_DEBUGS
	debugprintf("chartrans_transread: pc=%d\n",setp->pc) ;
#endif
	    if (setp->pc >= 0) {
		switch (setp->pc) {
		case charset_utf8:
	            rs = chartrans_transutf8(op,rcp,rcl,sp,sl) ;
		    break ;
		default:
	            rs = wsnwcpynarrow(rcp,rcl,sp,sl) ;
		    break ;
		} /* end switch */
	    } else {
	        int	ileft = sl ;
	        int	ostart = (rcl * sizeof(wchar_t)) ;
	        int	oleft ;
	        int	ofill ;
	        cchar	*ibp = sp ;
	        char	*obp = (char *) rcp ;
	        setp->acount = op->acount++ ;
	        oleft = ostart ;
	        rs = uiconv_trans(&setp->id,&ibp,&ileft,&obp,&oleft) ;
	        ofill = (ostart-oleft) ;
#if	CF_DEBUGS
	        debugprintf("chartrans_transread: uiconv_trans() rs=%d\n",rs) ;
	        debugprintf("chartrans_transread: oleft=%u ofill=%u\n",
	            oleft,ofill) ;
#endif
	        if (rs >= 0) {
	            rs = ((ofill & INT_MAX) >> 2) ;
	        }
	    } /* end if */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("chartrans_transread: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (chartrans_transread) */


/* private subroutines */


static int chartrans_setfins(CHARTRANS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if ((op->sets != NULL) && (op->nsets > 0)) {
	    int	si ;
	    for (si = 0 ; si < op->nmax ; si += 1) {
	        if (op->sets[si].name != NULL) {
	            rs1 = chartrans_setclose(op,si) ;
	            if (rs >= 0) rs = rs1 ;
	        }
	    } /* end for */
	} /* end if */

	return rs ;
}
/* end subroutine (chartrans_setfins) */


static int chartrans_setopen(CHARTRANS *op,time_t dt,int txid,cchar *np,int nl)
{
	CHARTRANS_SET	*setp = (op->sets + txid) ;
	int		rs ;
	cchar		*tcsp = CHARTRANS_NCS ;
	cchar		*name ;

	memset(setp,0,sizeof(CHARTRANS_SET)) ;

#if	CF_DEBUGS
	debugprintf("chartrans_setopen: ent id=%u n=%t\n",txid,np,nl) ;
#endif

	    if ((rs = uc_mallocstrw(np,nl,&name)) >= 0) {
	        int	pc ;
#if	CF_DEBUGS
	        debugprintf("chartrans_setopen: name=%s\n",name) ;
#endif
	            setp->name = name ;
	        if ((pc = matcasestr(charsets,np,nl)) >= 0) {
	            setp->pc = pc ;
	        } else {
	            const int	tlen = CSNLEN ;
	            char	tbuf[CSNLEN+1] = { 0 } ;
		    setp->pc = -1 ;
	            if ((rs = mktransname(tbuf,tlen,tcsp,-1)) >= 0) {
	                rs = uiconv_open(&setp->id,tbuf,name) ;
#if	CF_DEBUGS
			debugprintf("chartrans_setopen: uiconv_open() rs=%d\n",
			rs) ;
#endif
		    } /* end if (mktransname) */
	        }
#if	CF_DEBUGS
		debugprintf("chartrans_setopen: mid1 rs=%d pc=%d\n",rs,pc) ;
#endif
	        if (rs >= 0) {
	            setp->ti_access = dt ;
	            setp->acount = op->acount++ ;	/* time-stamp */
	            op->nsets += 1 ;
	        }
	        if (rs < 0) {
	            uc_free(name) ;
	            setp->name = NULL ;
	        }
#if	CF_DEBUGS
	debugprintf("chartrans_setopen: mid2 rs=%d pc=%d\n",rs,pc) ;
#endif
	    } /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("chartrans_setopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (chartrans_setopen) */


static int chartrans_setclose(CHARTRANS *op,int txid)
{
	CHARTRANS_SET	*setp = (op->sets + txid) ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("chartrans_setclose: ent id=%u n=%s\n",txid,setp->name) ;
#endif

	if (setp->name != NULL) {
	    if (setp->pc < 0) {
	        rs1 = uiconv_close(&setp->id) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(setp->name) ;
	    if (rs >= 0) rs = rs1 ;
	    setp->name = NULL ;
	    if (op->nsets > 0) op->nsets -= 1 ;
	}

#if	CF_DEBUGS
	debugprintf("chartrans_setclose: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (chartrans_setclose) */


static int chartrans_sethave(CHARTRANS *op,cchar *np,int nl)
{
	CHARTRANS_SET	*setp ;
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; i < op->nmax ; i += 1) {
	    setp = (op->sets + i) ;
	    if (setp->name != NULL) {
	        f = (strwcmp(setp->name,np,nl) == 0) ;
	        if (f) break ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("chartrans_sethave: f=%u i=%u\n",f,i) ;
#endif

	return (f) ? i : SR_NOTFOUND ;
}
/* end subroutine (chartrans_sethave) */


/* ARGSUSED */
static int chartrans_setfind(CHARTRANS *op,time_t dt)
{
	CHARTRANS_SET	*setp ;
	int		rs = SR_OK ;
	int		i ;
	int		mini = -1 ;
	int		acount = INT_MAX ;

	for (i = 0 ; i < op->nmax ; i += 1) {
	    setp = (op->sets + i) ;
	    if (setp->name != NULL) {
	        if ((setp->acount < acount) && (setp->uc == 0)) {
	            acount = setp->acount ;
	            mini = i ;
	        }
	    } else {
	        mini = i ;
	        break ;
	    }
	} /* end for */

	if (mini < 0) rs = SR_AGAIN ;

#if	CF_DEBUGS
	debugprintf("chartrans_setfind: rs=%d m=%u\n",rs,mini) ;
#endif

	return (rs >= 0) ? mini : rs ;
}
/* end subroutine (chartrans_setfind) */


static int chartrans_transutf8(CHARTRANS *op,wchar_t *rcp,int rcl,
		cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (rcl > 0) {
	    if (op->utf8decoder == NULL) {
	        const int	osize = sizeof(UTF8DECODER) ;
	        void		*p ;
	        if ((rs = uc_malloc(osize,&p)) >= 0) {
		    UTF8DECODER	*uop = (UTF8DECODER *) p ;
		    op->utf8decoder = p ;
		    rs = utf8decoder_start(uop) ;
		    if (rs < 0) {
		        uc_free(p) ;
		        op->utf8decoder = NULL ;
		    }
	        } /* end if (a-m) */
	    } /* end if (needed initialization) */
	    if (rs >= 0) {
	        UTF8DECODER	*uop = (UTF8DECODER *) op->utf8decoder ;
	        if ((rs = utf8decoder_load(uop,sp,sl)) > 0) {
		    if ((rs = utf8decoder_read(uop,rcp,rcl)) > 0) {
		        c += rs ;
			rcp += rs ;
			rcl -= rs ;
		    } /* end if */
	        } /* end if (utf8decoder_load) */
	    } /* end if (ok) */
	} /* end if (positive) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (chartrans_transutf8) */


static int mktransname(char *nbuf,int nlen,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(nbuf,nlen,i,np,nl) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    cchar	*suf = CHARTRANS_SUF ;
	    rs = storebuf_strw(nbuf,nlen,i,suf,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mktransname) */


