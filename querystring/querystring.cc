/* querystring */
/* lang=C++11 */

/* Query-String manager */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2017-09-25, David A­D­ Morano
	This module was adapted from some previous code.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We take a raw "query-string" and process it so that its components can
        be accessed.


*******************************************************************************/


#define	QUERYSTRING_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vector>
#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"querystring.h"


/* local defines */

#define	KEYVAL	struct keyval


/* name spaces */

using namespace	std ;


/* typedefs */


/* external subroutines */

extern "C" int	sfshrink(cchar *,int,cchar **) ;
extern "C" int	strwcmp(cchar *,cchar *,int) ;
extern "C" int	cfhexi(cchar *,int,int *) ;
extern "C" int	ishexlatin(int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
#endif

extern "C" char	*strwcpy(char *,cchar *,int) ;
extern "C" char	*strnchr(cchar *,int,int) ;
extern "C" char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */


/* local structures */

struct keyval {
	cchar		*kp = NULL ;
	cchar		*vp = NULL ;
	int		kl = 0 ;
	int		vl = 0 ;
	keyval(cchar *akp,int akl,cchar *avp,int avl) 
		: kp(akp), vp(avp), kl(akl), vl(avl) { 
	} ;
} ;

struct subinfo {
	QUERYSTRING	*op ;
	vector<keyval>	kvs ;
	char		*tbuf = NULL ;
	int		tlen = 0 ;
	subinfo(QUERYSTRING *aop) : op(aop) { } ;
	~subinfo() {
	    if (tbuf != NULL) {
		uc_free(tbuf) ;
		tbuf = NULL ;
		tlen = 0 ;
	    }
	    op = NULL ;
	} ;
	int tsize(int nlen) {
	     int	rs = 0 ;
	     if (nlen > tlen) {
	        if (tbuf != NULL) {
		    uc_free(tbuf) ;
		    tbuf = NULL ;
	        }
		tlen = nlen ;
		rs = uc_malloc((tlen+1),&tbuf) ;
	     }
	     return rs ;
	} ;
	int split(cchar *,int) ;
	int procpair(cchar *,int) ;
	int fixval(char *,int,cchar *,int) ;
	int load() ;
	int store(cchar *,int,cchar *,int) ;
} ;


/* forward references */

static char	*strwebhex(char *,cchar *,int) ;


/* local variables */


/* exported subroutines */


int querystring_start(QUERYSTRING *op,cchar *sp,int sl)
{
	const int	llen = MAXNAMELEN ;
	int		rs ;
	if (sl < 0) sl = strlen(sp) ;
	memset(op,0,sizeof(QUERYSTRING)) ;
	if ((rs = strpack_start(&op->packer,llen)) >= 0) {
	    subinfo	si(op) ;
	    op->open.packer = TRUE ;
	    if ((rs = si.split(sp,sl)) >= 0) {
		rs = si.load() ;
	    } /* end if (subinfo) */
	    if (rs < 0) {
	        op->open.packer = FALSE ;
		strpack_finish(&op->packer) ;
	    }
	} /* end if (strpack_start) */
	return rs ;
}
/* end subroutine (querystring_start) */


int querystring_finish(QUERYSTRING *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->kv != NULL) {
	    rs1 = uc_free(op->kv) ;
	    if (rs >= 0) rs = rs1 ;
	    op->kv = NULL ;
	}

	if (op->open.packer) {
	    op->open.packer = FALSE ;
	    rs1 = strpack_finish(&op->packer) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (querystring_finish) */


int querystring_count(QUERYSTRING *op)
{
	return op->n ;
}
/* end subroutine (querystring_count) */


int querystring_already(QUERYSTRING *op,cchar *kstr,int klen)
{
	const int	n = op->n ;
	int		i ;
	int		f = FALSE ;
	if (klen < 0) klen = strlen(kstr) ;
	for (i = 0 ; i < n ; i += 1) {
	    f = (strwcmp(op->kv[i][0],kstr,klen) == 0) ;
	    if (f) break ;
	}
	return f ;
}
/* end if (querystring_already) */


int querystring_curbegin(QUERYSTRING *op,QUERYSTRING_CUR *curp)
{
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (querystring_curbegin) */


int querystring_curend(QUERYSTRING *op,QUERYSTRING_CUR *curp)
{
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (querystring_curend) */


/* fetch the next entry value matching the given key */
int querystring_fetch(QUERYSTRING *op,cchar *kstr,int klen,
		QUERYSTRING_CUR *curp,cchar **rpp)
{
	int		rs = SR_OK ;
	int		vl = 0 ;

#if	CF_DEBUGS
	debugprintf("querystring_fetch: ent key=%t\n",kstr,klen) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (klen < 0) klen = strlen(kstr) ;

	if (op->n > 0) {
	    int		i = (curp->i + 1) ;
	    if (i < op->n) {
	        cchar	*(*kv)[2] = op->kv ;
	        int	f = FALSE ;
	        while (i < op->n) {
		    f = (strwcmp(kv[i][0],kstr,klen) == 0) ;
		    if (f) break ;
		    i += 1 ;
	        }
	        if (f) {
		    cchar	*vp = kv[i][1] ;
		    curp->i = i ;
		    vl = strlen(vp) ;
		    if (rpp != NULL) *rpp = vp ;
	        } else {
	            rs = SR_NOTFOUND ;
	        }
	    } else {
		rs = SR_NOTFOUND ;
	    }
	} else {
	    rs = SR_NOTFOUND ;
	}

#if	CF_DEBUGS
	debugprintf("querystring_fetch: ret rs=%d vl=%d\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (querystring_fetch) */


int querystring_enum(QUERYSTRING *op,QUERYSTRING_CUR *curp,
		cchar **kpp,cchar **vpp)
{
	int		rs = SR_OK ;
	int		vl = 0 ;

#if	CF_DEBUGS
	debugprintf("querystring_enum: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->n > 0) {
	    int		i = (curp->i + 1) ;
#if	CF_DEBUGS
	debugprintf("querystring_fetch: i=%d\n",i) ;
#endif
	    if (i < op->n) {
	        cchar	*(*kv)[2] = op->kv ;
		{
		    cchar	*kp = kv[i][0] ;
		    if (kpp != NULL) *kpp = kp ;
		}
	        {
		    cchar	*vp = kv[i][1] ;
		    vl = strlen(vp) ;
		    if (vpp != NULL) *vpp = vp ;
	        }
		    curp->i = i ;
	    } else {
		rs = SR_NOTFOUND ;
	    }
	} else {
	    rs = SR_NOTFOUND ;
	}

#if	CF_DEBUGS
	debugprintf("querystring_enum: ret rs=%d vl=%d\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (querystring_enum) */


/* private subroutines */


/* private subroutines */


int subinfo::split(cchar *sp,int sl)
{
	int		rs = SR_OK ;
	cchar		*tp ;
	while ((tp = strnchr(sp,sl,'&')) != NULL) {
	    const int	cl = (tp-sp) ;
	    cchar	*cp = sp ;
	    if (cl > 0) {
	        rs = procpair(cp,cl) ;
	    }
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	    if (rs < 0) break ;
	} /* end while */
	if ((rs >= 0) && (sl > 0)) {
	    rs = procpair(sp,sl) ;
	}
	return rs ;
}
/* end subroutine (subinfo::split) */


int subinfo::procpair(cchar *sbuf,int slen)
{
	int		rs = SR_OK ;
	int		sl ;
	cchar		*sp ;
	if (slen < 0) slen = strlen(sbuf) ;

#if	CF_DEBUGS
	debugprintf("subinfo::procpair: ent s=>%t<\n",sbuf,slen) ;
#endif

	if ((sl = sfshrink(sbuf,slen,&sp)) > 0) {
	    int		kl = sl ;
	    int		vl = 0 ;
	    cchar	*tp ;
	    cchar	*kp = sp ;
	    cchar	*vp = NULL ;
	    if ((tp = strnchr(sp,sl,'=')) != NULL) {
	        kl = (tp - sp) ;
	        while (kl && CHAR_ISWHITE(*kp)) kl -= 1 ;
	        vp = (tp + 1) ;
	        vl = ((sp + sl) - vp) ;
	    }
	    if (kl > 0) {
	        while ((vl > 0) && CHAR_ISWHITE(*vp)) {
	            vp += 1 ;
	            vl -= 1 ;
	        }
	        if ((vl > 0) && ((strnpbrk(vp,vl,"%+\t")) != NULL)) {
	            if ((rs = tsize(vl)) >= 0) {
	                rs = fixval(tbuf,tlen,vp,vl) ;
	                vl = rs ;
	                vp = tbuf ;
		    } /* end if (tsize) */
	        } /* end if (value) */
	        if (rs >= 0) {
	            rs = store(kp,kl,vp,vl) ;
	        }
	    } /* end if (key) */
	} /* end if (sfshrink) */

#if	CF_DEBUGS
	debugprintf("subinfo::procpair: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo::procpair) */


int subinfo::fixval(char *rbuf,int rlen,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		i = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo::fixval: ent v=>%t<\n",vp,vl) ;
#endif
	if (vl > 0) {
	    cchar	*tp ;
	    char	*rp = rbuf ;
	    if (vl > rlen) vl = rlen ;
	    while ((tp = strnpbrk(vp,vl,"%+\t")) != NULL) {
	        const int	sch = MKCHAR(*tp) ;
#if	CF_DEBUGS
	debugprintf("subinfo::fixval: sch=%c\n",sch) ;
#endif
	        if ((tp-vp) > 0) {
	            rp = strwcpy(rp,vp,(tp-vp)) ;
	        }
	        switch (sch) {
	        case '+':
	            if (((rp-rbuf) == 0) || (rp[-1] != ' ')) *rp++ = ' ' ;
	            break ;
	        case '\t':
	            if (((rp-rbuf) == 0) || (rp[-1] != ' ')) *rp++ = ' ' ;
	            break ;
	        case '%':
	            {
	                const int	tl = (vl-(tp-vp)) ;
#if	CF_DEBUGS
	                debugprintf("subinfo::fixval: tl=%d %=>%t<\n",
				tl,tp,tl) ;
#endif
	                rp = strwebhex(rp,tp,tl) ;
	                tp += MIN(2,tl) ;
	            }
	            break ;
	        } /* end switch */
	        vl -= ((tp+1)-vp) ;
	        vp = (tp+1) ;
	    } /* end while */
	    if ((rs >= 0) && (vl > 0)) {
	        while ((vl > 0) && CHAR_ISWHITE(vp[vl-1])) vl -= 1 ;
	        rp = strwcpy(rp,vp,vl) ;
	    }
	    i = (rp-rbuf) ;
	} /* end if (positive) */
	rbuf[i] = '\0' ;
#if	CF_DEBUGS
	debugprintf("subinfo::fixval: ret rs=%d i=%u\n",rs,i) ;
#endif
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (subinfo::fixval) */


int subinfo::store(cchar *kp,int kl,cchar *vp,int vl)
{
	STRPACK		*spp = &op->packer ;
	int		rs ;
	cchar		*sp ;
#if	CF_DEBUGS
	        debugprintf("subinfo::store: ent\n") ;
	        debugprintf("subinfo::store: kl=%d k=>%t<\n",kl,kp,kl) ;
	        debugprintf("subinfo::store: vl=%d v=>%t<\n",vl,vp,vl) ;
#endif
	if ((rs = strpack_store(spp,kp,kl,&sp)) >= 0) {
	    kp = sp ;
	    kl = rs ;
#if	CF_DEBUGS
	        debugprintf("subinfo::store: s kl=%d k=>%t<\n",kl,kp,kl) ;
#endif
	    if (vl > 0) {
	        if ((rs = strpack_store(spp,vp,vl,&sp)) >= 0) {
		    vp = sp ;
		    vl = rs ;
		}
	    } else {
		vp = (kp+kl) ;
#if	CF_DEBUGS
	        debugprintf("subinfo::store: z vl=%d v=>%s<\n",vl,vp) ;
#endif
	    }
	    if (rs >= 0) {
	        keyval	kv(kp,kl,vp,vl) ;
#if	CF_DEBUGS
	        debugprintf("subinfo::store: kv vraw=>%s<\n",vp) ;
	        debugprintf("subinfo::store: kv kl=%d k=>%t<\n",kl,kp,kl) ;
	        debugprintf("subinfo::store: kv vl=%d v=>%t<\n",vl,vp,vl) ;
#endif
	        kvs.push_back(kv) ;
	    } /* end if (ok) */
	} /* end if (strpack_store) */
	return rs ;
}
/* end subroutine (subinfo::store) */


int subinfo::load()
{
	const int	n = kvs.size() ;
	const int	esize = 2*sizeof(cchar *) ;
	int		rs ;
	int		size ;
	void		*p ;
#if	CF_DEBUGS
	debugprintf("subinfo::load: ent n=%d\n",n) ;
#endif
	size = ((n+1)*esize) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    op->kv = (cchar *(*)[2]) p ;
	    op->n = n ;
	    for (int i = 0 ; i < n ; i += 1) {
#if	CF_DEBUGS
	        debugprintf("subinfo::load: k=>%s<\n",kvs[i].kp) ;
	        debugprintf("subinfo::load: v=>%s<\n",kvs[i].vp) ;
#endif
	        op->kv[i][0] = kvs[i].kp ;
	        op->kv[i][1] = kvs[i].vp ;
	    }
	    op->kv[n][0] = NULL ;
	    op->kv[n][1] = NULL ;
	} /* end if (m-a) */
#if	CF_DEBUGS
	debugprintf("subinfo::load: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo::load) */


static char *strwebhex(char *rp,cchar *tp,int tl)
{
	if ((tl >= 3) && (*tp == '%')) {
	    const int	ch1 = MKCHAR(tp[1]) ;
	    const int	ch2 = MKCHAR(tp[2]) ;
	    if (ishexlatin(ch1) && ishexlatin(ch2)) {
	        int	v ;
	        if (cfhexi((tp+1),2,&v) >= 0) {
	            *rp++ = v ;
	        }
	    }
	}
	return rp ;
}
/* end subroutine (strwebhex) */


