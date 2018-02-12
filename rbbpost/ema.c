/* ema */

/* E-Mail Address */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		0		/* safety? */
#define	CF_ADDENT	1		/* enable 'ema_addent()' */
#define	CF_COMPACT	0		/* compact address on storage */
#define	CF_ALTCOMPACT	1		/* use alternative compacter */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an email address handling module object.  It can parse out and
	store hierarchically organized EMAs.


*******************************************************************************/


#define	EMA_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<ascii.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"ema.h"


/* local defines */

#undef	ASS
#define	ASS		struct ass

#undef	ADDRESSLEN
#define	ADDRESSLEN	60		/* starting address length */

#undef	N
#define	N		NULL


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif

extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */


/* local structures */

struct ass {
	char		*sp ;
	int		sl ;
	int		e ;
} ;

enum sis {
	si_address,
	si_route,
	si_comment,
	si_overlast
} ;


/* external subroutines */


/* forward references */

int		ema_starter(EMA *,cchar *,int) ;
int		ema_start(EMA *) ;
int		ema_parse(EMA *,cchar *,int) ;
int		ema_finish(EMA *) ;

static int	ema_parseit(EMA *,ASS *) ;
static int	ema_load(EMA *,cchar *,int,ASS *,EMA *) ;

#if	CF_ADDENT
static int	ema_addentone(EMA *,EMA_ENT *) ;
static int	ema_addents(EMA *,EMA *) ;
#endif

#if	CF_DEBUGS
static int	ema_debugprint(EMA *,cchar *) ;
#endif

static int	entry_start(EMA_ENT *) ;
static int	entry_finish(EMA_ENT *) ;

#if	CF_ADDENT
static int	entry_startload(EMA_ENT *,EMA_ENT *) ;
#endif

#if	CF_DEBUGS
static int	entry_debugprint(EMA_ENT *,cchar *) ;
#endif

#if	CF_COMPACT
static int	malloccompactstr(cchar *,int,char **) ;
#endif

static int	partsbegin(ASS *) ;
static int	partslen(ASS *) ;
static int	partsend(ASS *) ;

static int	ass_start(ASS *) ;
static int	ass_add(ASS *,int) ;
static int	ass_get(ASS *) ;
static int	ass_getprev(ASS *) ;
static int	ass_adv(ASS *) ;
static int	ass_skipwhite(ASS *) ;
static int	ass_backwhite(ASS *) ;
static int	ass_len(ASS *) ;
static int	ass_finish(ASS *) ;


/* local variables */

#if	CF_DEBUGS
static cchar	*statename[] = {
	"A",
	"R",
	"C",
	NULL
} ;
#endif /* CF_DEBUGS */


/* exported subroutines */


int ema_starter(EMA *hp,cchar *sp,int sl)
{
	int		rs ;

	if ((rs = ema_start(hp)) >= 0) {
	    rs = ema_parse(hp,sp,sl) ;
	    if (rs < 0)
	        ema_finish(hp) ;
	} /* end if (ema_start) */

	return rs ;
}
/* end subroutine (ema_starter) */


int ema_start(EMA *hp)
{
	const int	ne = EMADEFENTS ;
	int		rs ;

	if (hp == NULL) return SR_FAULT ;

#ifdef	OPTIONAL
	memset(hp,0,sizeof(EMA)) ;
#endif

	hp->magic = 0 ;
	hp->n = 0 ;
	if ((rs = vechand_start(&hp->list,ne,0)) >= 0) {
	    hp->magic = EMA_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("ema_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ema_start) */


/* free up this EMA object */
int ema_finish(EMA *hp)
{
	EMA_ENT		*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (hp == NULL) return SR_FAULT ;

	if (hp->magic != EMA_MAGIC) return SR_NOTOPEN ;

	hp->n = -1 ;
	for (i = 0 ; vechand_get(&hp->list,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {

	        rs1 = entry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;

#ifdef	COMMENT
	        if (hp->n > 0) hp->n -= 1 ;
#endif
	        rs1 = uc_free(ep) ;
	        if (rs >= 0) rs = rs1 ;

	    }
	} /* end for */

	rs1 = vechand_finish(&hp->list) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("ema_finish: ret rs=%d\n",rs) ;
#endif

	hp->magic = 0 ;
	return rs ;
}
/* end subroutine (ema_finish) */


/* parse out EMAs from a hostile environment */
int ema_parse(EMA *hp,cchar *sp,int sl)
{
	ASS		desc ;
	int		rs = SR_OK ;

	if (hp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (hp->magic != EMA_MAGIC) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("ema_parse: ent o=>%t<\n",sp,sl) ;
#endif

	desc.sp = (char *) sp ;
	desc.sl = sl ;
	if ((rs = ema_parseit(hp,&desc)) > 0) {
	    hp->n += rs ;
	}

#if	CF_DEBUGS
	debugprintf("ema_parse: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ema_parse) */


#if	CF_ADDENT
/* whew! this is not easy to implement (like any of this was)! */
int ema_addent(EMA *op,EMA_ENT *ep)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

	if (op->magic != EMA_MAGIC) return SR_NOTOPEN ;

	rs = ema_addentone(op,ep) ;

	return rs ;
}
/* end subroutine (ema_addent) */
#endif /* CF_ADDENT */


/* get the EMA under the current cursor */
int ema_get(EMA *hp,int i,EMA_ENT **epp)
{
	int		rs ;

	if (hp == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (hp->magic != EMA_MAGIC) return SR_NOTOPEN ;

	rs = vechand_get(&hp->list,i,epp) ;

#if	CF_DEBUGS
	debugprintf("ema_get: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ema_get) */


int ema_getbestaddr(EMA *hp,int i,cchar **rpp)
{
	EMA_ENT		*ep ;
	int		rs ;
	int		rl = 0 ;
	cchar		*rp = NULL ;

	if (hp == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	if (hp->magic != EMA_MAGIC) return SR_NOTOPEN ;

	if ((rs = vechand_get(&hp->list,i,&ep)) >= 0) {
	    if (ep != NULL) {
	    	if ((rl == 0) && (ep->rp != NULL) && (ep->rl > 0)) {
		    rp = ep->rp ;
		    rl = ep->rl ;
		}
	    	if ((rl == 0) && (ep->ap != NULL) && (ep->al > 0)) {
		    rp = ep->ap ;
		    rl = ep->al ;
		}
	    }
	} /* end if (get) */

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? rp : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("ema_getbestaddr: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (ema_getbestaddr) */


/* return the number of EMAs we have so far */
int ema_count(EMA *hp)
{
	int		rs ;

	if (hp == NULL) return SR_FAULT ;

	if (hp->magic != EMA_MAGIC) return SR_NOTOPEN ;

	rs = vechand_count(&hp->list) ;

#if	CF_DEBUGS
	debugprintf("ema_count: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ema_count) */


/* private subroutines */


/* this subroutine parses out EMAs recursively */
static int ema_parseit(EMA *hp,ASS *bp)
{
	ASS		as[si_overlast] ;
	EMA		*nlp = NULL ;
	int		rs = SR_OK ;
	int		size ;
	int		sl, olen ;
	int		pstate = si_address ;
	int		state ;
	int		ch ;
	int		n = 0 ;
	int		c_comment = 0 ;
	int		f_quote = FALSE ;
	int		f_exit = FALSE ;
	cchar		*orig ;

#if	CF_DEBUGS
	debugprintf("ema_parseit: ent len=%d\n",bp->sl) ;
	debugprintf("ema_parseit: orig=>%t<\n",bp->sp,bp->sl) ;
#endif /* CF_DEBUGS */

/* skip over any leading white space */

	ass_skipwhite(bp) ;

/* initialize for this entry */

	orig = bp->sp ;
	partsbegin(as) ;

/* start scanning */

	state = si_address ;
	while ((! f_exit) && ((ch = ass_get(bp)) >= 0)) {

#if	CF_DEBUGS
	    debugprintf("ema_parseit: C='%c' S=%s P=%s cl=%d q=%d\n",
	        ch,statename[state],
	        statename[pstate],c_comment,f_quote) ;
#endif

	    switch (ch) {

	    case '\\':
	        if (f_quote) {
	            ch = ass_adv(bp) ;
	            if (ch >= 0) {
	                ass_add((as + state),ch) ;
	                ass_adv(bp) ;
	            }
	        } else {
	            ass_add((as + state),ch) ;
	            ass_adv(bp) ;
	        }
	        break ;

	    case CH_DQUOTE:
	        ass_adv(bp) ;
	        f_quote = (! f_quote) ;
	        break ;

	    case CH_LPAREN:
	        if (! f_quote) {
	            if (c_comment == 0) {
#ifdef	COMMENT
	                size = ass_len(as + state) ;
	                if ((size > 0) && (ass_getprev(as + state) != ' '))
	                    ass_add((as + state),' ') ;
#endif
	                pstate = state ;
	                state = si_comment ;
	                size = ass_len(as + state) ;
	                if ((size > 0) && (ass_getprev(as + state) != ' ')) {
	                    ass_add((as + state),' ') ;
	                }
	                ass_adv(bp) ;
	            } else {
	                ass_add((as + state),ch) ;
	                ass_adv(bp) ;
	            }
	            c_comment += 1 ;
	        } else {
	            ass_add((as + state),ch) ;
	            ass_adv(bp) ;
	        }
	        break ;

	    case CH_RPAREN:
	        if ((! f_quote) && (c_comment > 0)) {
	            c_comment -= 1 ;
	            if (c_comment == 0) {
	                state = pstate ;
	                ass_adv(bp) ;
	            } else {
	                ass_add((as + state),ch) ;
	                ass_adv(bp) ;
	            }
	        } else {
	            ass_add((as + state),ch) ;
	            ass_adv(bp) ;
	        }
	        break ;

	    case '<':
	        if ((state == si_address) && 
	            (! f_quote) && (c_comment == 0)) {
	            pstate = state ;
	            state = si_route ;
	            ass_adv(bp) ;
	        } else {
	            ass_add((as + state),ch) ;
	            ass_adv(bp) ;
	        }
	        break ;

	    case '>':
	        if ((state == si_route) && 
	            (! f_quote) && (c_comment == 0)) {
	            state = pstate ;
	            ass_adv(bp) ;
	        } else {
	            ass_add((as + state),ch) ;
	            ass_adv(bp) ;
	        }
	        break ;

	    case ':':
	        if ((state == si_address) &&
	            (! f_quote) && (c_comment == 0)) {
	            size = sizeof(EMA) ;
	            if ((rs = uc_malloc(size,&nlp)) >= 0) {
	                if ((rs = ema_start(nlp)) >= 0) {
	                    ass_adv(bp) ;
	                    rs = ema_parseit(nlp,bp) ;
	                    if (rs < 0)
	                        ema_finish(nlp) ;
	                }
	                if (rs < 0) {
	                    uc_free(nlp) ;
	                    nlp = NULL ;
	                }
	            } /* end if (allocation) */
	        } else {
	            ass_add((as + state),ch) ;
	            ass_adv(bp) ;
	        }
	        break ;

	    case ';':
	        if ((state == si_address) && 
	            (! f_quote) && (c_comment == 0)) {
	            f_exit = TRUE ;
	            olen = (bp->sp - orig) ;
	            ass_adv(bp) ;
	        } else {
	            ass_add((as + state),ch) ;
	            ass_adv(bp) ;
	        }
	        break ;

	    case ',':
	        if ((! f_quote) && (c_comment == 0) && 
	            (state == si_address)) {
	            olen = bp->sp - orig ;
	            if ((olen > 0) && (partslen(as) > 0)) {
	                n += 1 ;
	                rs = ema_load(hp,orig,olen,as,nlp) ;
	            }
#if	CF_DEBUGS
	            ema_debugprint(hp,"1") ;
#endif
	            partsend(as) ;
	            if (rs >= 0) {
	                nlp = NULL ;
	                partsbegin(as) ;
	                ass_adv(bp) ;
	                ass_skipwhite(bp) ;
	                orig = bp->sp ;
	                state = si_address ;
	            } /* end if */
#if	CF_DEBUGS
	            ema_debugprint(hp,"2") ;
#endif
	        } else {
	            ass_add((as + state),ch) ;
	            ass_adv(bp) ;
	        }
	        break ;

/* I think that these cases are just some optimizations (not required) */
	    case '\n':
	    case '\r':
	    case '\v':
	    case '\f':
	    case '\t':
	    case ' ':
	    case MKCHAR(CH_NBSP):
#if	CF_DEBUGS
	        debugprintf("ema_parseit: LWSP\n") ;
#endif
	        if (! f_quote) {
#if	CF_DEBUGS
	            debugprintf("ema_parseit: no-quote\n") ;
#endif
	            if (c_comment == 0) {
#if	CF_DEBUGS
	                debugprintf("ema_parseit: no-comment\n") ;
#endif
	                int	ai = state ;
	                if ((state == si_route) || (state == si_address)) {
#if	CF_DEBUGS
	                    debugprintf("ema_parseit: state={A|R}\n") ;
#endif
	                    ass_adv(bp) ;
	                    break ;
	                } else if ((as[ai].sl == 0) ||
	                    (as[ai].sp[as[ai].sl-1] == ' ')) {
#if	CF_DEBUGS
	                    debugprintf("ema_parseit: special\n") ;
#endif
	                    ch = 0 ;
	                    ass_adv(bp) ;
	                    break ;
	                } /* end if */
	            } else {
#if	CF_DEBUGS
	                debugprintf("ema_parseit: comment=%u\n",
	                    c_comment) ;
#endif
	                sl = ass_len(as+si_comment) ;
#if	CF_DEBUGS
	                {
	                    const int pch = ass_getprev(as+si_comment) ;
	                    debugprintf("ema_parseit: "
	                        "sl=%u pch=>%c<(\\x%02x)\n",
	                        sl,pch,pch) ;
	                }
#endif /* CF_DEBUGS */
	                if ((sl == 0) || 
	                    (CHAR_ISWHITE(ass_getprev(as+si_comment)))) {
#if	CF_DEBUGS
	                    debugprintf("ema_parseit: skip\n") ;
#endif
	                    ch = 0 ;
	                    ass_adv(bp) ;
	                    break ;
	                } /* end if */
	            } /* end if */
	        } /* end if (not in a quote) */

/* FALLTHROUGH */
	    default:
#if	CF_DEBUGS
	        debugprintf("ema_parseit: default state=%u ch=>%c<\n",
	            state,ch) ;
#endif
	        if (ch > 0) {
	            if (c_comment) {
	                ass_add((as + si_comment),ch) ;
	            } else {
	                ass_add((as + state),ch) ;
	            }
	        }
	        ass_adv(bp) ;
	        break ;

	    } /* end switch */

#if	CF_DEBUGS
	    debugprintf("ema_parseit: switch-end rs=%d state=%u\n",rs,state) ;
#endif

	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("ema_parseit: while-end rs=%d\n",rs) ;
#endif

#ifdef	COMMENT /* not needed here */
	if (rs >= 0) {
	    ASS		*asp = (as+si_comment) ;
	    if (ass_len(asp) > 0) {
	        int	pch = ass_getprev(asp) ;
	        if (CHAR_ISWHITE(pch)) ass_backwhite(asp) ;
	    }
	}
#endif /* COMMENT */

	if (! f_exit) {
	    olen = (bp->sp - orig) ;
	}

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("ema_parseit: finishing\n") ;
	    debugprintf("ema_parseit: comment=>%t<\n",
	        as[si_comment].sp,as[si_comment].sl) ;
	    for (i = 0 ; i < as[si_comment].sl ; i += 1) {
	        debugprintf("ema_parseit: comment ch=>%c<\n",
	            as[si_comment].sp[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

	if (rs >= 0) {
	    if ((olen > 0) && (partslen(as) > 0)) {

#if	CF_DEBUGS
	        debugprintf("ema_parseit: doing the last one\n") ;
	        debugprintf("ema_parseit: address=>%t<\n",
	            as[si_address].sp,as[si_address].sl) ;
	        debugprintf("ema_parseit: route=>%t<\n",
	            as[si_route].sp,as[si_route].sl) ;
#endif /* CF_DEBUGS */

	        n += 1 ;
	        rs = ema_load(hp,orig,olen,as,nlp) ;
	        nlp = NULL ;

#if	CF_DEBUGS
	        ema_debugprint(hp,"after") ;
#endif

	    } else {

	        if (nlp != NULL) {
	            ema_finish(nlp) ;
	            uc_free(nlp) ;
	            nlp = NULL ;
	        }

	    } /* end if */

	} /* end if */

	if (rs < 0) {
	    if (nlp != NULL) {
	        ema_finish(nlp) ;
	        uc_free(nlp) ;
	    }
	}

	partsend(as) ;

	hp->n += n ;

#if	CF_DEBUGS
	ema_debugprint(hp,"ret") ;
#endif

#if	CF_DEBUGS
	debugprintf("ema_parseit: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ema_parseit) */


static int ema_load(EMA *hp,cchar *orig,int olen,ASS *as,EMA *nlp)
{
	EMA_ENT		*ep = NULL ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("ema_load: ent olen=%d orig=>%t<\n",olen,orig,olen) ;
#endif

	if (olen < 0)
	    olen = strlen(orig) ;

#if	CF_DEBUGS
	debugprintf("ema_load: mod olen=%d\n",olen) ;
#endif

	if (olen > 0) {
	    const int	size = sizeof(EMA_ENT) ;
	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    if ((rs = entry_start(ep)) >= 0) {
	        int	i ;
	        int	sl ;
	        cchar	*sp ;
	        cchar	*cp ;
	        for (i = 0 ; (rs >= 0) && (i < si_overlast) ; i += 1) {

#if	CF_DEBUGS
	            debugprintf("ema_load: i=%d\n",i) ;
#endif

	            if ((as[i].sp != NULL) && (as[i].sp[0] != '\0')) {

#if	CF_DEBUGS
	                debugprintf("ema_load: i=%u l=%d >%s<\n",
	                    i,as[i].sl,as[i].sp) ;
#endif

	                sp = as[i].sp ;
	                sl = as[i].sl ;
	                if (sl < 0) sl = strlen(sp) ;

	                switch (i) {

	                case si_address:
#if	CF_DEBUGS
	                    debugprintf("ema_load: address=>%t<\n",sp,sl) ;
#endif
	                    if ((sl >= 1) &&
	                        ((sp[0] == '~') || (sp[0] == '_'))) {
#if	CF_DEBUGS
	                        debugprintf("ema_load: PCS list\n") ;
#endif
	                        sp += 1 ;
	                        sl -= 1 ;
	                        ep->type = ematype_pcs ;
	                    } /* end if (PCS list-type) */
	                    if ((rs = uc_mallocstrw(sp,sl,&cp)) >= 0) {
	                        ep->ap = cp ;
	                        ep->al = sl ;
	                    }
	                    break ;

	                case si_route:
#if	CF_DEBUGS
	                    debugprintf("ema_load: route=>%t<\n",sp,sl) ;
#endif
	                    if ((rs = uc_mallocstrw(sp,sl,&cp)) >= 0) {
	                        ep->rp = cp ;
	                        ep->rl = sl ;
	                    }
	                    break ;

	                case si_comment:
#if	CF_DEBUGS
	                    debugprintf("ema_load: comment=>%t<\n",sp,sl) ;
#endif
	                    if ((rs = uc_mallocstrw(sp,sl,&cp)) >= 0) {
	                        ep->cp = cp ;
	                        ep->cl = sl ;
	                    }
	                    break ;

	                } /* end switch */

#if	CF_DEBUGS
	                {
	                    debugprintf("ema_load: i=%u rs=%d len=%d\n",
	                        i,rs,sl) ;
	                    if (sl >= 0)
	                        debugprintf("ema_load: c=>%t<\n",cp,sl) ;
	                }
#endif /* CF_DEBUGS */

	            } /* end if */

	        } /* end for */

#if	CF_DEBUGS
	        debugprintf("ema_load: mid rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) && (olen > 0)) {
	            while ((olen > 0) && CHAR_ISWHITE(orig[olen-1])) {
	                olen -= 1 ;
	            }
	            if ((rs = uc_mallocstrw(orig,olen,&cp)) >= 0) {
	                ep->op = cp ;
	                ep->ol = olen ;
	            }
#if	CF_DEBUGS
	            debugprintf("ema_load: original=>%s<\n",ep->op) ;
#endif
	        } /* end if */

#if	CF_DEBUGS
	        debugprintf("ema_load: far rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) && (nlp != NULL)) {
	            ep->listp = nlp ;
	            ep->type = ematype_group ;
	        }
#if	CF_DEBUGS
	        debugprintf("ema_load: loading entry into list\n") ;
	        entry_debugprint(ep,"before_load") ;
#endif

	        if (rs >= 0) {
	            if ((rs = vechand_add(&hp->list,ep)) >= 0) {
	                ep = NULL ;
	                hp->n += 1 ;
	            }
	        }
	        if (rs < 0)
	            entry_finish(ep) ;
	    } /* end if (entry_start) */
	    if (rs < 0)
	        uc_free(ep) ;
	} /* end if (memory-allocation) */
	} /* end if (positive) */

#if	CF_DEBUGS
	if ((rs >= 0) && (ep != NULL))
	    entry_debugprint(ep,"ret") ;
	debugprintf("ema_load: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ema_load) */


#if	CF_ADDENT

static int ema_addentone(EMA *op,EMA_ENT *ep)
{
	EMA_ENT		*nep ;
	const int	size = sizeof(EMA_ENT) ;
	int		rs ;

	if ((rs = uc_malloc(size,&nep)) >= 0) {
	    if ((rs = entry_startload(nep,ep)) >= 0) {
	        if ((rs = vechand_add(&op->list,nep)) >= 0) {
	            op->n += 1 ;
	        }
	        if (rs < 0)
	            entry_finish(nep) ;
	    }
	    if (rs < 0)
	        uc_free(nep) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (ema_addentone) */


static int ema_addents(EMA *op,EMA *oop)
{
	EMA_ENT		*oep ;
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; vechand_get(&oop->list,i,&oep) >= 0 ; i += 1) {
	    rs = ema_addentone(op,oep) ;
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (ema_addents) */

#endif /* CF_ADDENT */


#if	CF_DEBUGS
static int ema_debugprint(EMA *hp,cchar *s)
{
	EMA_ENT		*ep ;
	int		i ;
	if (s != NULL)
	    debugprintf("ema_debugprint: s=%s\n",s) ;
	debugprintf("ema_debugprint: n=%u\n",hp->n) ;
	for (i = 0 ; vechand_get(&hp->list,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
	    entry_debugprint(ep,s) ;
	} /* end for */
	return SR_OK ;
}
/* end subroutine (ema_debugprint) */
#endif /* CF_DEBUGS */


static int entry_start(EMA_ENT *ep)
{

	memset(ep,0,sizeof(EMA_ENT)) ;
	ep->type = ematype_reg ;
	return SR_OK ;
}
/* end subroutine (entry_start) */


static int entry_finish(EMA_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (ep == NULL) return SR_FAULT ;
#endif

	if (ep->op != NULL) {
	    rs1 = uc_free(ep->op) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->op = NULL ;
	}

	if (ep->ap != NULL) {
	    rs1 = uc_free(ep->ap) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->ap = NULL ;
	}

	if (ep->rp != NULL) {
	    rs1 = uc_free(ep->rp) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->rp = NULL ;
	}

	if (ep->cp != NULL) {
	    rs1 = uc_free(ep->cp) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->cp = NULL ;
	}

	if (ep->listp != NULL) {
	    rs1 = ema_finish(ep->listp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(ep->listp) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->listp = NULL ;
	} /* end if (recursive free-up) */

#ifdef	OPTIONAL
	memset(ep,0,sizeof(EMA_ENT)) ;
#endif

	return rs ;
}
/* end subroutine (entry_finish) */


#if	CF_ADDENT

static int entry_startload(EMA_ENT *ep,EMA_ENT *oep)
{
	int		rs = SR_OK ;

	*ep = *oep ;
	if (oep->op != NULL)
	    ep->op = mallocstrw(oep->op,oep->ol) ;

	if (oep->ap != NULL)
	    ep->ap = mallocstrw(oep->ap,oep->al) ;

	if (oep->rp != NULL)
	    ep->rp = mallocstrw(oep->rp,oep->rl) ;

	if (oep->cp != NULL)
	    ep->cp = mallocstrw(oep->cp,oep->cl) ;

	if (oep->listp != NULL) {
	    EMA		*nop = NULL ; /* LINT assignment */
	    const int	size = sizeof(EMA) ;
	    ep->listp = NULL ;
	    if ((rs = uc_malloc(size,&nop)) >= 0) {
	        if ((rs = ema_start(nop)) >= 0) {
	            if ((rs = ema_addents(nop,oep->listp)) >= 0) {
	                ep->listp = nop ;
	            }
	            if (rs < 0)
	                ema_finish(nop) ;
	        }
	        if (rs < 0)
	            uc_free(nop) ;
	    } /* end if (allocation) */
	} /* end if (non-NULL) */

	if (rs < 0)
	    entry_finish(ep) ;

	return rs ;
}
/* end subroutine (entry_startload) */

#endif /* CF_ADDENT */


#if	CF_DEBUGS
static int entry_debugprint(EMA_ENT *ep,cchar *s)
{
#if	CF_SAFE
	if (ep == NULL) return SR_FAULT ;
#endif
	if (s != NULL)
	    debugprintf("entry_debugprint: s=%s\n",s) ;
	debugprintf("entry_debugprint: type=%u\n",ep->type) ;
	if (ep->op)
	    debugprintf("entry_debugprint: O %u %t\n",
	        ep->ol,ep->op,ep->ol) ;
	if (ep->ap)
	    debugprintf("entry_debugprint: A %u %t\n",
	        ep->al,ep->ap,ep->al) ;
	if (ep->rp)
	    debugprintf("entry_debugprint: R %u %t\n",
	        ep->rl,ep->rp,ep->rl) ;
	if (ep->cp)
	    debugprintf("entry_debugprint: C %u %t\n",
	        ep->cl,ep->cp,ep->cl) ;
	return SR_OK ;
}
/* end subroutine (entry_debugprint) */
#endif /* CF_DEBUGS */


#if	CF_COMPACT

#if	CF_ALTCOMPACT

static int malloccompactstr(cchar *sbuf,int sl,char **rpp)
{
	int		rs ;
	int		size ;
	int		len = 0 ;
	int		f_quote = FALSE ;
	const uchar	*sp = (const uchar *) sbuf ;
	uchar		*buf ;

	if (sl < 0)
	    sl = strlen(sbuf) ;

	size = (sl + 1) ;
	if ((rs = uc_malloc(size,&buf)) >= 0) {
	    int		ch ;
	    uchar	*bp = buf ;

	    while (sl > 0) {

	        ch = MKCHAR(*sp) ;
	        switch (ch) {

	        case CH_DQUOTE:
	            f_quote = (! f_quote) ;

/* FALLTHROUGH */
	        default:
	            if (f_quote || (! CHAR_ISWHITE(ch))) {
	                *bp++ = ch ;
	            }
	            break ;

	        } /* end switch */

	        sp += 1 ;
	        sl -= 1 ;

	    } /* end while */

	    *bp = '\0' ;
	    len = (bp - buf) ;

	} /* end if (memory-allocation) */

	*rpp = (rs >= 0) ? buf : NULL ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (malloccompactstr) */

#else /* CF_ALTCOMPACT */

static int malloccompactstr(cchar *sbuf,int sl,char **rpp)
{
	ASS		s ;
	int		rs = SR_OK ;
	int		ch ;
	int		len ;
	int		f_quote = FALSE ;
	const uchar	*sp = (uchar *) sbuf ;

	ass_start(&s) ;

	if (sl < 0)
	    sl = strlen(sbuf) ;

#if	CF_DEBUGS
	debugprintf("malloccompactstr: ent >%t<\n",buf,buflen) ;
#endif

	while ((rs >= 0) && (sl > 0)) {

#if	CF_DEBUGS
	    debugprintf("malloccompactstr: switch >%c<\n",(*sp & 0xFF)) ;
#endif

	    ch = (*sp & 0xff) ;
	    switch (ch) {
	    case CH_DQUOTE:
	        f_quote = (! f_quote) ;
/* FALLTHROUGH */
	    default:
	        if (f_quote || (! CHAR_ISWHITE(ch))) {
#if	CF_DEBUGS
	            debugprintf("malloccompactstr: adding >%c<\n",ch) ;
#endif
	            rs = ass_add(&s,ch) ;
	        }
	        break ;
	    } /* end switch */

	    sp += 1 ;
	    sl -= 1 ;

	} /* end while */
	len = s.sl ;

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? mallocstrw(s.sp,s.sl) : NULL ;
	}

	ass_finish(&s) ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (malloccompactstr) */

#endif /* CF_ALTCOMPACT */

#endif /* CF_COMPACT */


static int partsbegin(ASS *asp)
{
	int		i ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;
#endif

	for (i = 0 ; i < si_overlast ; i += 1) {
	    ass_start(asp + i) ;
	}

	return SR_OK ;
}
/* end subroutine (partsbegin) */


/* calculate the combined length of the all of the subparts */
static int partslen(ASS *asp)
{
	int		rs1 ;
	int		i ;
	int		len = 0 ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;
#endif

	for (i = 0 ; i < si_overlast ; i += 1) {
	    ass_backwhite(asp+i) ;
	    rs1 = ass_len(asp+i) ;
	    if (rs1 >= 0) len += rs1 ;
	} /* end for */

	return len ;
}
/* end subroutine (partslen) */


static int partsend(ASS *asp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;
#endif

	for (i = 0 ; i < si_overlast ; i += 1) {
	    rs1 = ass_finish(asp + i) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (partsend) */


static int ass_start(ASS *asp)
{

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;
#endif

	asp->sp = NULL ;
	asp->sl = 0 ;
	asp->e = 0 ;
	return SR_OK ;
}
/* end subroutine (ass_start) */


static int ass_finish(ASS *asp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;
#endif

	if (asp->sp != NULL) {
	    rs1 = uc_free(asp->sp) ;
	    if (rs >= 0) rs = rs1 ;
	    asp->sp = NULL ;
	}

	asp->sl = 0 ;
	asp->e = 0 ;
	return rs ;
}
/* end subroutine (ass_finish) */


static int ass_add(ASS *asp,int ch)
{
	int		rs = SR_OK ;
	int		ne ;
	int		len = 0 ;
	char		*p ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;
#endif

	if (asp->sp == NULL) {
	    asp->sl = 0 ;
	    ne = ADDRESSLEN ;
	    if ((rs = uc_malloc((ne + 1),&p)) >= 0) {
	        asp->sp = p ;
	        asp->e = ne ;
	    }
	}

	if ((rs >= 0) && (asp->e == asp->sl)) {
	    ne = (asp->e + ADDRESSLEN) ;
	    if ((rs = uc_realloc(asp->sp,(ne + 1),&p)) >= 0) {
	        asp->sp = p ;
	        asp->e = ne ;
	    }
	}

	if (rs >= 0) {
	    asp->sp[(asp->sl)++] = ch ;
	    asp->sp[asp->sl] = '\0' ;
	    len = asp->sl ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (ass_add) */


static int ass_get(ASS *asp)
{
	int		rs = SR_OK ;
	int		ch ;

	if (asp->sl > 0) {
	    ch = (asp->sp[0] & 0xff) ;
	} else
	    rs = SR_EOF ;

	return (rs >= 0) ? ch : rs ;
}
/* end subroutine (ass_get) */


static int ass_getprev(ASS *asp)
{
	int		rs = SR_OK ;
	int		ch ;

	if (asp->sl > 0) {
	    ch = (asp->sp[asp->sl-1] & 0xff) ;
	} else
	    rs = SR_EOF ;

	return (rs >= 0) ? ch : rs ;
}
/* end subroutine (ass_getprev) */


/* advance one character forward in the string */
static int ass_adv(ASS *asp)
{
	int		rs = SR_OK ;
	int		ch ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;
#endif

	if (asp->sl > 0) {
	    asp->sp += 1 ;
	    asp->sl -= 1 ;
	}
	if (asp->sl > 0) {
	    ch = (asp->sp[0] & 0xff) ;
	} else
	    rs = SR_EOF ;

	return (rs >= 0) ? ch : rs ;
}
/* end subroutine (ass_adv) */


static int ass_len(ASS *asp)
{

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;
#endif

	return asp->sl ;
}
/* end subroutine (ass_len) */


/* this is not really a method of 'ass', this subroutine breaks the object */
static int ass_skipwhite(ASS *asp)
{
	int		i = 0 ;

#if	CF_SAFE
	if (asp->sp == NULL) return SR_FAULT ;
#endif

	while ((asp->sl != 0) && CHAR_ISWHITE(*(asp->sp))) {
	    asp->sp += 1 ;
	    if (asp->sl > 0) asp->sl -= 1 ;
	    i += 1 ;
	} /* end while */

	return i ;
}
/* end subroutine (ass_skipwhite) */


static int ass_backwhite(ASS *asp)
{
	int		f = FALSE ;
	while (asp->sl > 0) {
	    int	lch = asp->sp[asp->sl-1] ;
	    f = CHAR_ISWHITE(lch) ;
	    if (! f) break ;
	    asp->sl -= 1 ;
	} /* end while */
	return f ;
}
/* end subroutine (ass_backwhite) */


