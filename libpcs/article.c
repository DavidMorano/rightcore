/* article */

/* manage an ARTICLE object */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SAFE		1		/* safety */


/* revision history:

	= 1995-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object manages the particulars about an article.


*******************************************************************************/


#define	ARTICLE_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"article.h"


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


#ifdef	COMMENT
struct article_flags {
	uint		path:1 ;
	uint		envdate:1 ;
	uint		msgdate:1 ;
	uint		jobfile:1 ;
	uint		nopath:1 ;
	uint		ngs:1 ;
} ;
struct article_head {
	RETPATH		path ;
	DATER		envdate ;
	DATER		msgdate ;
	NG		ngs ;
	EMA		addrfrom ;
	EMA		addrsender ;
	EMA		addrto ;
	EMA		addrbcc ;
	struct article_flags	f ;
	const char	*envfrom ;
	const char	*messageid ;
	const char	*subject ;
	const char	*articleid ;
	const char	*ngdname ;
} ;
#endif /* COMMENT */

int article_start(ARTICLE *op)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	memset(op,0,sizeof(ARTICLE)) ;
	op->clen = -1 ;
	op->clines = -1 ;

	if ((rs = ng_start(&op->ngs)) >= 0) {
	    op->f.ngs = TRUE ;
	    if ((rs = retpath_start(&op->path)) >= 0) {
	        op->f.path = TRUE ;
		if ((rs = vechand_start(&op->envdates,1,0)) >= 0) {
	            op->f.envdates = TRUE ;
		}
		if (rs < 0) {
	            op->f.path = TRUE ;
	    	    retpath_finish(&op->path) ;
		}
	    }
	    if (rs < 0) {
	        op->f.ngs = FALSE ;
		ng_finish(&op->ngs) ;
	    }
	} /* end if (ngs) */
 
	return rs ;
}
/* end subroutine (article_start) */


int article_finish(ARTICLE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("article_finish: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("article_finish: f_msgdate=%u\n",op->f.msgdate) ;
#endif
	if (op->f.msgdate) {
	    op->f.msgdate = FALSE ;
	    rs1 = dater_finish(&op->msgdate) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("article_finish: f_envdates=%u\n",op->f.envdates) ;
#endif
	if (op->f.envdates) {
	    DATER	*dp ;
	    int		i ;
	    op->f.envdates = FALSE ;
	    for (i = 0 ; vechand_get(&op->envdates,i,&dp) >= 0 ; i += 1) {
		rs1 = dater_finish(dp) ;
		if (rs >= 0) rs = rs1 ;
		rs1 = uc_free(dp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end for */
	    rs1 = vechand_finish(&op->envdates) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (envdates) */

#if	CF_DEBUGS
	debugprintf("article_finish: f_path=%u\n",op->f.path) ;
#endif
	if (op->f.path) {
	    op->f.path = FALSE ;
	    rs1 = retpath_finish(&op->path) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("article_finish: f_ngs=%u\n",op->f.ngs) ;
#endif
	if (op->f.ngs) {
	    op->f.ngs = FALSE ;
	    rs1 = ng_finish(&op->ngs) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("article_finish: addrs\n") ;
#endif
	for (i = 0 ; i < articleaddr_overlast ; i += 1) {
	    if (op->af[i]) {
	        op->af[i] = FALSE ;
	        rs1 = ema_finish(&op->addr[i]) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */
#if	CF_DEBUGS
	debugprintf("article_finish: strs\n") ;
#endif

	for (i = 0 ; i < articlestr_overlast ; i += 1) {
	    if (op->strs[i] != NULL) {
	        rs1 = uc_free(op->strs[i]) ;
	        if (rs >= 0) rs = rs1 ;
	        op->strs[i] = NULL ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("article_finish: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (article_finish) */


int article_addpath(ARTICLE *op,const char *sp,int sl)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (! op->f.path) {
	    op->f.path = TRUE ;
	    rs = retpath_start(&op->path) ;
	}

	if (rs >= 0) {
	    rs = retpath_parse(&op->path,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (article_addpath) */


int article_addng(ARTICLE *op,const char *sp,int sl)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (! op->f.ngs) {
	    op->f.ngs = TRUE ;
	    rs = ng_start(&op->ngs) ;
	}

	if (rs >= 0) {
	    rs = ng_addparse(&op->ngs,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (article_addng) */


int article_addenvdate(ARTICLE *op,DATER *d2p)
{
	DATER		*dp ;
	const int	msize = sizeof(DATER) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (d2p == NULL) return SR_FAULT ;

	if ((rs = uc_malloc(msize,&dp)) >= 0) {
	    if ((rs = dater_startcopy(dp,d2p)) >= 0) {
		rs = vechand_add(&op->envdates,dp) ;
		if (rs < 0)
		    dater_finish(dp) ;
	    }
	    if (rs < 0)
		uc_free(dp) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (article_addenvdate) */


int article_addmsgdate(ARTICLE *op,DATER *dp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (dp == NULL) return SR_FAULT ;

	if (! op->f.msgdate) {
	    op->f.msgdate = TRUE ;
	    rs = dater_start(&op->msgdate,NULL,NULL,0) ;
	}

	if (rs >= 0) {
	    rs = dater_setcopy(&op->msgdate,dp) ;
	}

	return rs ;
}
/* end subroutine (article_addmsgdate) */


int article_addaddr(ARTICLE *op,int type,const char *sp,int sl)
{
	const int	n = articleaddr_overlast ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if ((type < 0) || (type >= n)) return SR_INVALID ;

	if (! op->af[type]) {
	    op->af[type] = TRUE ;
	    rs = ema_start(&op->addr[type]) ;
	}

	if (rs >= 0) {
	    rs = ema_parse(&op->addr[type],sp,sl) ;
	}

	return rs ;
}
/* end subroutine (article_addaddr) */


int article_addstr(ARTICLE *op,int type,const char *sp,int sl)
{
	const int	n = articlestr_overlast ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if ((type < 0) || (type >= n)) return SR_INVALID ;

	if (op->strs[type] != NULL) {
	    rs = uc_free(op->strs[type]) ;
	    op->strs[type] = NULL ;
	}

	if (rs >= 0) {
	    const char	*cp ;
	    if ((rs = uc_mallocstrw(sp,sl,&cp)) > 0) {
		op->strs[type] = cp ;
		rs = (rs-1) ;
	    }
	}

	return rs ;
}
/* end subroutine (article_addstr) */


#ifdef	COMMENT
int article_add(ngp,ngbuf,nglen,ngdname)
NG		*ngp ;
const char	ngbuf[] ;
int		nglen ;
const char	ngdname[] ;
{
	NG_ENT		ne ;
	int		rs = SR_OK ;

#if	CF_SAFE
	if (ngp == NULL)
	    return SR_FAULT ;
#endif

	if (nglen >= 0) {
	    ne.len = nglen ;
	} else {
	    ne.len = strlen(ngbuf) ;
	}

	ne.name = (const char *) mallocstrw(ngbuf,ne.len) ;

	ne.dir = NULL ;
	if (ngdname != NULL) {
	    ne.dir = (const char *) mallocstr(ngdname) ;
	}

	rs = vecitem_add(ngp,&ne,sizeof(NG_ENT)) ;

	return rs ;
}
/* end subroutine (article_add) */
#endif /* COMMENT */


/* extract newsgroup names from the "newsgroups" header string */
int article_addparse(ARTICLE *op,cchar *sp,int sl)
{
	EMA		aid ;
	EMA_ENT		*ep ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("article_addparse: ent\n") ;
	debugprintf("article_addparse: > %t\n",sp,sl) ;
#endif

	if ((rs = ema_start(&aid)) >= 0) {
	    if ((rs = ema_parse(&aid,sp,sl)) > 0) {
		int	i ;
		int	cl ;
		cchar	*cp ;

#if	CF_DEBUGS
	        debugprintf("article_addparse: got some ema\n") ;
#endif

	        for (i = 0 ; ema_get(&aid,i,&ep) >= 0 ; i += 1) {
	            if (ep != NULL) {

#if	CF_DEBUGS
	            debugprintf("article_addparse: ema entry\n") ;
#endif

	            if ((ep->f.error) || (ep->al <= 0)) continue ;

	            if ((cl = sfshrink(ep->ap,ep->al,&cp)) > 0) {
			n += 1 ;
	                rs = ng_add(&op->ngs,cp,cl,NULL) ;
		    }

		    }
	            if (rs < 0) break ;
	        } /* end for */

	    } /* end if (parse) */
	    rs1 = ema_finish(&aid) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ema) */

#if	CF_DEBUGS
	debugprintf("article_addparse: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (article_addparse) */


int article_ao(ARTICLE *op,uint aoff,uint alen)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	op->aoff = aoff ;
	op->alen = alen ;
	return SR_OK ;
}
/* end subroutine (article_ao) */


int article_countenvdate(ARTICLE *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = vechand_count(&op->envdates) ;
	return rs ;
}
/* end subroutine (article_countenvdate) */


int article_getenvdate(ARTICLE *op,int i,DATER **epp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = vechand_get(&op->envdates,i,epp) ;
	return rs ;
}
/* end subroutine (article_getenvdate) */


int article_getstr(ARTICLE *op,int type,cchar **rpp)
{
	const int	n = articlestr_overlast ;
	int		rs = SR_OK ;
	const char	*sp ;

	if (op == NULL) return SR_FAULT ;

	if ((type < 0) || (type >= n)) return SR_INVALID ;

	sp = op->strs[type] ;
	if (sp == NULL) rs = SR_NOTFOUND ;

	if (rs >= 0) rs = strlen(sp) ;

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? sp : NULL ;
	}

	return rs ;
}
/* end subroutine (article_getstr) */


int article_getaddrema(ARTICLE *op,int type,EMA **rpp)
{
	const int	n = articleaddr_overlast ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if ((type < 0) || (type >= n)) return SR_INVALID ;

	if (! op->af[type]) {
	    op->af[type] = TRUE ;
	    rs = ema_start(&op->addr[type]) ;
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? (op->addr + type) : NULL ;
	}

	return rs ;
}
/* end subroutine (article_getaddrema) */


