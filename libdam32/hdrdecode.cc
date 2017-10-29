/* hdrdecode */
/* lang=C++98 */

/* mail-header weirdo-string decoding */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This was really made from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We facilitate decoding of mail-headers (coded parts of them).

		hdrdecode_start
		hdrdecode_proc
		hdrdecode_finish


*******************************************************************************/


#define	HDRDECODE_MASTER	0	/* necessary for proper symbol names */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<stddef.h>
#include	<string.h>

#include	<string>
#include	<new>

#include	<vsystem.h>
#include	<toxc.h>
#include	<localmisc.h>

#include	"hdrdecode.h"


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif

#define	SUBINFO		class subinfo
#define	SUBINFO_FL	class subinfo_flags

#define	ESCINFO		struct escinfo

#ifndef	CHARSETNAMELEN
#define	CHARSETNAMELEN	60
#endif


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

extern "C" int	snwcpyuc(char *,int,cchar *,int) ;
extern "C" int	wsnwcpynarrow(wchar_t *,int,cchar *,int) ;
extern "C" int	sfshrink(cchar *,int,cchar **) ;
extern "C" int	sisub(cchar *,int,cchar *) ;
extern "C" int	sichr(cchar *,int,int) ;
extern "C" int	matcasestr(cchar **,cchar *,int) ;
extern "C" int	isprintbad(int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	debugprinthexblock(cchar *,int,const void *,int) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" char	*strwcpy(char *,const char *,int) ;
extern "C" char	*strwset(char *,int,int) ;
extern "C" char	*strnchr(const char *,int,int) ;
extern "C" char	*strnsub(const char *,int,cchar *) ;


/* external variables */


/* local structures */

struct escinfo {
	cchar		*csp ;			/* charset specification */
	cchar		*edp ;			/* encoded data */
	int		csl ;			/* charset specification */
	int		edl ;			/* encoded data */
	int		ech ;			/* encoding (character) */
	int		skip ;			/* skip length */
} ;

class subinfo {
	HDRDECODE	*op ;
	int		rlen ;
	int		rl ;
	wchar_t		*rarr ;
	cchar		*sp ;
	int		sl ;
	int procreg(int) ;
	int procreg(cchar *,int) ;
	int procreger(cchar *,int) ;
	int proctrans(ESCINFO *) ;
	int proctranser(ESCINFO *,cchar *,int) ;
	int proctrans_b(ESCINFO *,char *,int) ;
	int proctrans_q(ESCINFO *,char *,int) ;
	int proctrans_unknown(ESCINFO *,char *,int,cchar *) ;
	int store(int ch) {
	    int	c = 0 ;
	    if ((rlen-rl) > 0) {
	        rarr[rl++] = ch ;
	        c = 1 ;
	    }
	    return c ;
	} ;
	int store(cchar *cp,int cl) {
	    int	c = 0 ;
	    int	i ;
	    for (i = 0 ; (i < cl) && cp[i] ; i += 1) {
	        int	ch = (int) MKCHAR(cp[i]) ;
	        c += store(ch) ;
	    }
	    return c ;
	} ;
	int store(wchar_t *wp,int wl) {
	    int	c = 0 ;
	    int	i ;
	    for (i = 0 ; (i < wl) && wp[i] ; i += 1) {
	        int	ch = (int) wp[i] ;
	        c += store(ch) ;
	    }
	    return c ;
	} ;
	int storetrans(int,cchar *,int) ;
public:
	subinfo(HDRDECODE *aop,wchar_t *ararr,int arlen) : 
	    op(aop), rlen(arlen), rl(0), rarr(ararr) {
	} ;
	int begin(cchar *asp,int asl) {
	    sp = asp ;
	    sl = asl ;
	    return SR_OK ;
	} ;
	int end() {
	    return SR_OK ;
	} ;
	int proc() ;
} ;


/* forward references */

static int	hdrdecode_b64decoder(HDRDECODE *) ;
static int	hdrdecode_qpdecoder(HDRDECODE *) ;
static int	hdrdecode_chartrans(HDRDECODE *) ;

static int	escinfo_have(ESCINFO *,cchar *,int) ;
static int	escinfo_skip(ESCINFO *) ;
static int	escinfo_pass(ESCINFO *) ;


/* local variables */

static const char	*passes[] = {
	"iso-8859-1",
	"iso-Latin-1",
	"Latin-1",
	"us-ascii",
	"ascii",
	NULL
} ;


/* exported subroutines */


int hdrdecode_start(HDRDECODE *op,cchar *pr)
{
	int		rs ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("hdrdecode_start: ent pr=%s\n",pr) ;
#endif

	memset(op,0,sizeof(HDRDECODE)) ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    op->pr = cp ;
	    op->magic = HDRDECODE_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("hdrdecode_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (hdrdecode_start) */


int hdrdecode_finish(HDRDECODE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != HDRDECODE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("hdrdecode_finish: ent\n") ;
#endif

	if (op->chartrans) {
	    rs1 = chartrans_close(op->chartrans) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(op->chartrans) ;
	    if (rs >= 0) rs = rs1 ;
	    op->chartrans = NULL ;
	}
	if (op->b64decoder) {
	    rs1 = b64decoder_finish(op->b64decoder) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(op->b64decoder) ;
	    if (rs >= 0) rs = rs1 ;
	    op->b64decoder = NULL ;
	}
	if (op->qpdecoder) {
	    rs1 = qpdecoder_finish(op->qpdecoder) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(op->qpdecoder) ;
	    if (rs >= 0) rs = rs1 ;
	    op->qpdecoder = NULL ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("hdrdecode_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (hdrdecode_finish) */


int hdrdecode_proc(HDRDECODE *op,wchar_t *rarr,int rlen,cchar *sp,int sl)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("hdrdecode_proc: ent rlen=%u\n",rlen) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (rarr == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (op->magic != HDRDECODE_MAGIC) return SR_NOTOPEN ;

	if (rlen < 0) return SR_INVALID ;

	if (sl < 0) sl = strlen(sp) ;

	if (strnsub(sp,sl,"=?") != NULL) {
	    subinfo	s(op,rarr,rlen) ;
	    if ((rs = s.begin(sp,sl)) >= 0) {
	        {
	            rs = s.proc() ;
	            c = rs ;
	        }
	        rs1 = s.end() ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (s) */
	} else {
	    rs = wsnwcpynarrow(rarr,rlen,sp,sl) ;
	    c += rs ;
	}

#if	CF_DEBUGS
	debugprintf("hdrdecode_proc: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (hdrdecode_proc) */


/* private subroutines */


static int hdrdecode_b64decoder(HDRDECODE *op)
{
	int		rs = SR_OK ;
	if (op->b64decoder == NULL) {
	    const int	size = sizeof(B64DECODER) ;
	    void	*p ;
#if	CF_DEBUGS
	    debugprintf("hdrdecode_b64decoder: initializing\n") ;
#endif
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        op->b64decoder = (B64DECODER *) p ;
	        rs = b64decoder_start(op->b64decoder) ;
	        if (rs < 0) {
	            uc_free(op->b64decoder) ;
	            op->b64decoder = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (initialization needed) */
	return rs ;
}
/* end subroutine (hdrdecode_b64decoder) */


static int hdrdecode_qpdecoder(HDRDECODE *op)
{
	int		rs = SR_OK ;
	if (op->qpdecoder == NULL) {
	    const int	size = sizeof(QPDECODER) ;
	    void	*p ;
#if	CF_DEBUGS
	    debugprintf("hdrdecode_qpdecoder: init\n") ;
#endif
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        op->qpdecoder = (QPDECODER *) p ;
	        rs = qpdecoder_start(op->qpdecoder,TRUE) ;
	        if (rs < 0) {
	            uc_free(op->qpdecoder) ;
	            op->qpdecoder = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (initialization needed) */
	return rs ;
}
/* end subroutine (hdrdecode_qpdecoder) */


static int hdrdecode_chartrans(HDRDECODE *op)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("hdrdecode_chartrans: ent\n") ;
#endif
	if (op->chartrans == NULL) {
	    const int	size = sizeof(CHARTRANS) ;
	    void	*p ;
#if	CF_DEBUGS
	    debugprintf("hdrdecode_chartrans: init\n") ;
#endif
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        op->chartrans = (CHARTRANS *) p ;
	        rs = chartrans_open(op->chartrans,op->pr,2) ;
#if	CF_DEBUGS
	        debugprintf("hdrdecode_chartrans: chartrans_open() rs=%d\n",
			rs) ;
#endif
	        if (rs < 0) {
	            uc_free(op->chartrans) ;
	            op->chartrans = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (initialization needed) */
#if	CF_DEBUGS
	debugprintf("hdrdecode_chartrans: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (hdrdecode_chartrans) */


int subinfo::proc()
{
	ESCINFO		ei ;
	int		rs = SR_OK ;
	int		si ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo_proc: ent s=>%t<\n",
	    sp,strlinelen(sp,sl,40)) ;
#endif
	while ((si = escinfo_have(&ei,sp,sl)) >= 0) {
	    if ((rs = procreg(si)) >= 0) {
	        c += rs ;
	        if ((rs = proctrans(&ei)) >= 0) {
	            c += rs ;
	            rs = escinfo_skip(&ei) ;
#if	CF_DEBUGS
	            debugprintf("subinfo_proc: escinfo_skip() rs=%d\n",rs) ;
	            debugprintf("subinfo_proc: sl=%u\n",sl) ;
	            debugprintf("subinfo_proc: s=»%t«\n",
	                sp,strlinelen(sp,sl,40)) ;
#endif
	            sp += rs ;
	            sl -= rs ;
	        } /* end if (proctrans) */
	    } /* end if (procreg) */
	    if (rs < 0) break ;
	} /* end while */
	if ((rs >= 0) && (sl > 0)) {
#if	CF_DEBUGS
	    debugprintf("subinfo_proc: rem=>%t<\n",sp,sl) ;
#endif
	    if ((rs = procreg(sl)) >= 0) {
	        c += rs ;
	    }
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_proc) */


int subinfo::procreg(int n)
{
	return procreger(sp,n) ;
}
/* end subroutine (subinfo_procreg) */


int subinfo::procreg(cchar *ep,int n)
{
	return procreger(ep,n) ;
}
/* end subroutine (subinfo_procreg) */


int subinfo::procreger(cchar *ep,int n)
{
	int		i ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo_procreger: e=>%t<\n",ep,n) ;
#endif
	for (i = 0 ; (i < n) && ep[i] ; i += 1) {
	    int	ch = MKCHAR(ep[i]) ;
	    c += store(ch) ;
	} /* end for */
	rarr[rl] = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo_procreger: ret c=%u\n",c) ;
#endif
	return c ;
}
/* end subroutine (subinfo_procreger) */


int subinfo::proctrans(ESCINFO *eip)
{
	int		rs = SR_OK ;
	int		el = eip->edl ;
	int		c = 0 ;

#if	CF_DEBUGS
	{
	    cchar	*ep = eip->edp ;
	    debugprintf("subinfo::proctrans: ent el=%u\n",el) ;
	    debugprintf("subinfo::proctrans: e=>%t<\n",
	        ep,strlinelen(ep,el,40)) ;
	}
#endif

	if (el > 0) {
	    int		tlen = (eip->edl+3) ; /* allow for error indicator */
	    cchar	*keystr = "BQ" ;
	    char	*tbuf ;
	    if ((rs = uc_malloc((tlen+1),&tbuf)) >= 0) {
	        const int	ki = sichr(keystr,-1,eip->ech) ;
	        int		tl = 0 ;
	        int		f_err = FALSE ;

#if	CF_DEBUGS
	        {
	            debugprintf("subinfo::proctrans: ech=%c (%u)\n",
	                eip->ech,ki) ;
	            debugprintf("subinfo::proctrans: charset=>%t<\n",
	                eip->csp,eip->csl) ;
	            debugprintf("subinfo::proctrans: data=>%t<\n",
	                eip->edp,eip->edl) ;
	        }
#endif /* CF_DEBUGS */

	        switch (ki) {
	        case 0: /* B */
	            rs = proctrans_b(eip,tbuf,tlen) ;
	            tl = rs ;
	            break ;
	        case 1: /* Q */
	            rs = proctrans_q(eip,tbuf,tlen) ;
	            tl = rs ;
	            break ;
	        default:
	            f_err = TRUE ;
	            rs = proctrans_unknown(eip,tbuf,tlen,"«¿»") ;
	            tl = rs ;
	            break ;
	        } /* end switch */

#if	CF_DEBUGS
	        debugprintf("subinfo::proctrans: mid1 rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) && (tl > 0)) {
	            if ((escinfo_pass(eip) >= 0) || f_err) {
	                rs = store(tbuf,tl) ;
	                c += rs ;
	            } else {
	                rs = proctranser(eip,tbuf,tl) ;
	                c += rs ;
	            }
	        } /* end if (pass or trans) */

	        uc_free(tbuf) ;
	    } /* end if (m-a-f) */
	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("subinfo::proctrans: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo::proctrans) */


int subinfo::proctranser(ESCINFO *eip,cchar *tp,int tl)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo::proctranser: ent\n") ;
#endif
	if ((rs = hdrdecode_chartrans(op)) >= 0) {
	    CHARTRANS		*ctp = op->chartrans ;
	    const time_t	dt = time(NULL) ;
	    const int		nlen = CHARSETNAMELEN ;
	    char		nbuf[CHARSETNAMELEN+1] ;
#if	CF_DEBUGS
	    debugprintf("subinfo::proctranser: _chartrans() rs=%d\n",rs) ;
#endif
	    if ((rs = snwcpyuc(nbuf,nlen,eip->csp,eip->csl)) >= 0) {
	        const int	cel = rs ;
	        cchar		*cep = nbuf ;
	        if ((rs = chartrans_transbegin(ctp,dt,cep,cel)) >= 0) {
	            const int	txid = rs ;
	            {
	                rs = storetrans(txid,tp,tl) ;
	                c = rs ;
	            }
	            rs1 = chartrans_transend(ctp,txid) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (chartrans-transaction) */
	    } /* end if (snwcpyuc) */
	} /* end if (hdrdecode_chartrans) */
#if	CF_DEBUGS
	debugprintf("subinfo::proctranser: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo::proctranser) */


int subinfo::storetrans(int txid,cchar *tp,int tl)
{
	const int	rlen = (tl*sizeof(wchar_t)) ;
	int		rs ;
	int		wl = 0 ;
	wchar_t		*rbuf ;
#if	CF_DEBUGS
	debugprintf("subinfo::storetrans: ent tl=%d\n",tl) ;
	debugprinthexblock("subinfo::storetrans",80,tp,tl) ;
#endif
	if ((rs = uc_malloc((rlen+1),&rbuf)) >= 0) {
	    CHARTRANS	*ctp = op->chartrans ;
	    if ((rs = chartrans_transread(ctp,txid,rbuf,rlen,tp,tl)) >= 0) {
	        rs = store(rbuf,rs) ;
	        wl = rs ;
	    }
	    uc_free(rbuf) ;
	} /* end if (m-a) */
#if	CF_DEBUGS
	debugprintf("subinfo::storetrans: ret rs=%d wl=%d\n",rs,wl) ;
#endif
	return (rs >= 0) ? wl : rs ;
}
/* end subroutine (subinfo::storetrans) */


int subinfo::proctrans_b(ESCINFO *eip,char *tbuf,int tlen)
{
	int		rs ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo_proctransb: ent\n") ;
#endif
	if ((rs = hdrdecode_b64decoder(op)) >= 0) {
	    B64DECODER	*dp = op->b64decoder ;
	    const int	el = eip->edl ;
	    cchar	*ep = eip->edp ;
#if	CF_DEBUGS
	    debugprintf("subinfo_proctransb: e=>%t<\n",
	        ep,strlinelen(ep,el,40)) ;
#endif
	    if ((rs = b64decoder_load(dp,ep,el)) >= 0) {
	        int	tl = tlen ;
	        char	*tp = tbuf ;
	        while ((tl > 0) && ((rs = b64decoder_read(dp,tp,tl)) > 0)) {
	            tp += rs ;
	            tl -= rs ;
	            c += rs ;
	        } /* end while */
	    } /* end if (b64decoder_load) */
	} /* end if (hdrdecode_b64decoder) */
#if	CF_DEBUGS
	debugprintf("subinfo_proctransb: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo::proctrans_b) */


int subinfo::proctrans_q(ESCINFO *eip,char *tbuf,int tlen)
{
	int		rs ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo_proctransq: ent\n") ;
#endif
	if ((rs = hdrdecode_qpdecoder(op)) >= 0) {
	    QPDECODER	*dp = op->qpdecoder ;
	    const int	el = eip->edl ;
	    cchar	*ep = eip->edp ;
#if	CF_DEBUGS
	    debugprintf("subinfo_proctransq: e=>%t<\n",ep,el) ;
#endif
	    if ((rs = qpdecoder_load(dp,ep,el)) >= 0) {
	        int	tl = tlen ;
	        char	*tp = tbuf ;
	        while ((tl > 0) && ((rs = qpdecoder_read(dp,tp,tl)) > 0)) {
	            tp += rs ;
	            tl -= rs ;
	            c += rs ;
	        } /* end while */
	    } /* end if (qpdecoder_load) */
	} /* end if (hdrdecode_qpdecoder) */
#if	CF_DEBUGS
	debugprintf("subinfo_proctransq: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo::proctrans_q) */


/* ARGSUSED */
int subinfo::proctrans_unknown(ESCINFO *eip,char *tbuf,int tlen,cchar *sp)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	int		tl = tlen ;
	char		*tp = tbuf ;
	while ((tl > 0) && *sp) {
	    *tp++ = *sp ;
	    tl -= 1 ;
	    c += 1 ;
	} /* end while */
	if (*sp != '\0') rs = SR_OVERFLOW ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo::proctrans_unknown) */


/* do we have a coding sequence? */
static int escinfo_have(ESCINFO *eip,cchar *sp,int sl)
{
	int		si ;
	memset(eip,0,sizeof(ESCINFO)) ;
	if ((si = sisub(sp,sl,"=?")) >= 0) {
	    int		cl ;
	    cchar	*sp_start = sp ;
	    eip->csp = sp+(si+2) ;
	    sp += (si+2) ;
	    sl -= (si+2) ;
	    if ((cl = sichr(sp,sl,'?')) >= 0) {
	        int	i ;
	        eip->csl = cl ;
	        sp += (cl+1) ;
	        sl -= (cl+1) ;
	        if ((i = sichr(sp,sl,'?')) >= 0) {
	            cchar	*chp ;
	            eip->edp = (sp + (i+1)) ;
	            if (sfshrink(sp,i,&chp) > 0) {
	                int	ei ;
	                eip->ech = touc(*chp) ;
	                sp += (i+1) ;
	                sl -= (i+1) ;
	                if ((ei = sisub(sp,sl,"?=")) >= 0) {
	                    eip->skip = ((sp+ei+2)-sp_start) ;
	                    eip->edl = (sp+ei-eip->edp) ;
	                } else {
	                    si = -1 ;
	                }
	            } else {
	                si = -1 ;
	            }
	        } else {
	            si = -1 ;
	        }
	    } else {
	        si = -1 ;
	    }
	} /* end if (have escape) */
	return si ;
}
/* end subroutine (escinfo_have) */


static int escinfo_skip(ESCINFO *eip)
{
	return (eip->skip) ;
}
/* end subroutine (escinfo_skip) */


static int escinfo_pass(ESCINFO *eip)
{
	return matcasestr(passes,eip->csp,eip->csl) ;
}
/* end subroutine (escinfo_pass) */


