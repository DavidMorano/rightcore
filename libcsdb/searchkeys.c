/* searchkeys */

/* provides management for search-keys */


#define	CF_DEBUGS 	0		/* compile-time debug print-outs */
#define	CF_SAFE		0		/* some safety */
#define	CF_ASSERT	0		/* use some assertions */
#define	CF_REGPROC	0		/* compile regular processing */
#define	CF_BUILDREDUCE	1		/* try '_buildreduce()' */
#define	CF_SHORTCUT	1		/* use short-cut */
#define	CF_KPHRASE	0		/* debug |kphrase_xxx()| */
#define	CF_CHECKIT	0		/* debug |_checkit()| */


/* revision history:

	= 2009-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages groups of words (called 'phrases') that form search
	keys for searching a text database.  The text database is not our
	concern as we only get a call to us (with a word) to confirm whether
	all keys have been matched or not.  We maintain whether all of the
	search keys have been matched or not and return this indication to the
	caller.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<strpack.h>
#include	<localmisc.h>

#include	"searchkeys.h"
#include	"naturalwords.h"
#include	"xwords.h"


/* local defines */

#undef	BUILD
#define	BUILD		struct build

#undef	BUILDPHRASE
#define	BUILDPHRASE	struct build_phrase

#undef	SKPHRASE
#define	SKPHRASE	struct searchkeys_kphrase

#undef	SKWORD
#define	SKWORD		struct searchkeys_kword

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	NATURALWORDLEN
#endif


/* external subroutines */

extern int	snwcpy(char *,int,cchar *) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	hasuc(cchar *,int) ;
extern int	isprintlatin(int) ;
extern int	isalnumlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* local structures */

struct build {
	vecobj	phrases ;
} ;

struct build_phrase {
	vecobj	words ;
} ;


/* forward references */

static int searchkeys_build(SEARCHKEYS *,cchar **) ;
static int searchkeys_buildadd(SEARCHKEYS *,BUILD *,cchar *) ;
static int searchkeys_buildaddword(SEARCHKEYS *,BUILDPHRASE *,cchar *,int) ;
static int searchkeys_buildphrasemat(SEARCHKEYS *,BUILD *,BUILDPHRASE *) ;
static int searchkeys_buildmatone(SEARCHKEYS *,BUILD *,int,BUILDPHRASE *) ;
static int searchkeys_buildreduce(SEARCHKEYS *,BUILD *) ;
static int searchkeys_buildload(SEARCHKEYS *,BUILD *) ;
static int searchkeys_buildfins(SEARCHKEYS *,BUILD *) ;
static int searchkeys_curinc(SEARCHKEYS *,SEARCHKEYS_CUR *,int *,int *) ;

#if	CF_DEBUGS && CF_CHECKIT
static int searchkeys_checkit(SEARCHKEYS *,uint,cchar *) ;
#endif

static int buildphrase_start(BUILDPHRASE *) ;
static int buildphrase_add(BUILDPHRASE *,cchar *,int) ;
static int buildphrase_count(BUILDPHRASE *) ;
static int buildphrase_getkey(BUILDPHRASE *,int,cchar **) ;
static int buildphrase_havekey(BUILDPHRASE *,cchar *,int) ;
static int buildphrase_finish(BUILDPHRASE *) ;

#if	CF_REGPROC
static int kphrase_process(SKPHRASE *,int,int,cchar *,int) ;
#endif

static int kphrase_processxw(SKPHRASE *,int,int,XWORDS *) ;


/* local variables */


/* exported subroutines */


int searchkeys_start(SEARCHKEYS *op,cchar**qsp)
{
	int		rs ;
	int		nphrases = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (qsp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("searchkeys_start: qs¬\n") ;
	    for (i = 0 ; qsp[i] != NULL ; i += 1) {
	        debugprintf("searchkeys_start: qs=>%s<\n",qsp[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

	memset(op,0,sizeof(SEARCHKEYS)) ;

	if ((rs = strpack_start(&op->stores,0)) >= 0) {
	    if ((rs = searchkeys_build(op,qsp)) >= 0) {
	        nphrases = op->nphrases ;
	        op->magic = SEARCHKEYS_MAGIC ;
	    }
	    if (rs < 0)
	        strpack_finish(&op->stores) ;
	} /* end if (strpack) */

#if	CF_DEBUGS
	debugprintf("searchkeys_start: ret rs=%d np=%u\n",rs,nphrases) ;
#endif

	return (rs >= 0) ? nphrases : rs ;
}
/* end subroutine (searchkeys_start) */


int searchkeys_finish(SEARCHKEYS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SEARCHKEYS_MAGIC) return SR_NOTOPEN ;

	if (op->kphrases != NULL) {
	    int	i ;

	    for (i = 0 ; i < op->nphrases ; i += 1) {
	        if (op->kphrases[i].kwords != NULL) {
	            rs1 = uc_free(op->kphrases[i].kwords) ;
	            if (rs >= 0) rs = rs1 ;
	            op->kphrases[i].kwords = NULL ;
	        }
	    } /* end for */

	    rs1 = uc_free(op->kphrases) ;
	    if (rs >= 0) rs = rs1 ;
	    op->kphrases = NULL ;
	    op->nphrases = 0 ;

	} /* end if */

	rs1 = strpack_finish(&op->stores) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (searchkeys_finish) */


int searchkeys_popbegin(SEARCHKEYS *op,SEARCHKEYS_POP *pop,int f_prefix)
{
	int		rs = SR_OK ;
	int		size ;
	int		n = 0 ;
	void		*p ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SEARCHKEYS_MAGIC) return SR_NOTOPEN ;

#if	CF_SAFE
	if (pop == NULL) return SR_FAULT ;
#endif

	if (op->nphrases <= 0) return SR_INVALID ;

#if	defined(OPTIONAL) || 1
	memset(pop,0,sizeof(SEARCHKEYS_POP)) ;
#endif

	pop->f_prefix = f_prefix ;
	n = op->nphrases ;
	size = (n + 1) * sizeof(int) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    pop->nmatch = (int *) p ;
	    memset(pop->nmatch,0,size) ;
	    pop->cphrases = n ;
	    pop->magic = SEARCHKEYS_MAGIC ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (searchkeys_popbegin) */


int searchkeys_popend(SEARCHKEYS *op,SEARCHKEYS_POP *pop)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SEARCHKEYS_MAGIC) return SR_NOTOPEN ;

#if	CF_SAFE
	if (pop == NULL) return SR_FAULT ;
#endif

	if (pop->magic != SEARCHKEYS_MAGIC) return SR_NOTOPEN ;

	pop->cphrases = 0 ;
	if (pop->nmatch != NULL) {
	    rs1 = uc_free(pop->nmatch) ;
	    if (rs >= 0) rs = rs1 ;
	    pop->nmatch = NULL ;
	}

	pop->magic = 0 ;
	return rs ;
}
/* end subroutine (searchkeys_popend) */


#if	CF_REGPROC

int searchkeys_process(SEARCHKEYS *op,SEARCHKEYS_POP *pop,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		f_match = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (pop == NULL) return SR_FAULT ;

	if (op->magic != SEARCHKEYS_MAGIC) return SR_NOTOPEN ;
	if (pop->magic != SEARCHKEYS_MAGIC) return SR_NOTOPEN ;

	if (pop->nmatch == NULL) return SR_BADFMT ;

	if (pop->cphrases > 0) {
	    SKPHRASE	*pep ;
	    int		ki ;
	    int		i ;
	    int		f_prefix = pop->f_prefix ;
	    for (i = 0 ; i < op->nphrases ; i += 1) {
	        pep = (op->kphrases + i) ;
	        ki = pop->nmatch[i] ;
	        if (ki < pep->nwords) {
	            if ((rs = kphrase_process(pep,f_prefix,ki,sp,sl)) > 0) {
	                pop->nmatch[i] += 1 ;
	                if (pop->nmatch[i] == pep->nwords) {
	                    pop->cphrases -= 1 ;
		        }
	            } else if (rs == 0) {
	                pop->nmatch[i] = 0 ;
		    }
	        }
	    } /* end for */
	    if (pop->cphrases == 0) f_match = TRUE ;
	} else {
	    f_match = TRUE ;
	}

	return (rs >= 0) ? f_match : rs ;
}
/* end subroutine (searchkeys_process) */

#endif /* CF_REGPROC */


int searchkeys_processxw(SEARCHKEYS *op,SEARCHKEYS_POP *pop,XWORDS *xwp)
{
	int		rs = SR_OK ;
	int		f_match = FALSE ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (pop == NULL) return SR_FAULT ;
	if (xwp == NULL) return SR_FAULT ;

	if (op->magic != SEARCHKEYS_MAGIC) return SR_NOTOPEN ;

	if (pop->nmatch == NULL) return SR_BADFMT ;
#endif /* CF_SAFE */

	if (pop->cphrases > 0) {
	    SKPHRASE	*pep ;
	    int		ki ;
	    int		i ;
	    int		f_prefix = pop->f_prefix ;
	    for (i = 0 ; (rs >= 0) && (i < op->nphrases) ; i += 1) {
	        pep = (op->kphrases + i) ;
	        ki = pop->nmatch[i] ;
	        if (ki < pep->nwords) {
	            if ((rs = kphrase_processxw(pep,f_prefix,ki,xwp)) > 0) {
	                pop->nmatch[i] += 1 ;
	                if (pop->nmatch[i] == pep->nwords) {
	                    pop->cphrases -= 1 ;
		        }
	            } else {
	                pop->nmatch[i] = 0 ;
		    }
	        } /* end if */
	    } /* end for */
	    if (pop->cphrases == 0) f_match = TRUE ;
	} else {
	    f_match = TRUE ;
	}

#if	CF_DEBUGS && CF_CHECKIT
	searchkeys_checkit(op,tid,"processxw-ret") ;
#endif
#if	CF_DEBUGS
	debugprintf("searchkeys_processxw: ret id=%u i=%u\n",tid,i) ;
	debugprintf("searchkeys_processxw: ret rs=%d f_mt=%u\n",
	    rs,f_match) ;
#endif

	return (rs >= 0) ? f_match : rs ;
}
/* end subroutine (searchkeys_processxw) */


int searchkeys_curbegin(SEARCHKEYS *op,SEARCHKEYS_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SEARCHKEYS_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	curp->j = 0 ;

	return SR_OK ;
}
/* end subroutine (searchkeys_curbegin) */


int searchkeys_curend(SEARCHKEYS *op,SEARCHKEYS_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SEARCHKEYS_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	curp->j = 0 ;

	return SR_OK ;
}
/* end subroutine (searchkeys_curend) */


int searchkeys_enum(SEARCHKEYS *op,SEARCHKEYS_CUR *curp,cchar **rpp)
{
	int		rs ;
	int		i, j ;
	int		kl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SEARCHKEYS_MAGIC) return SR_NOTOPEN ;

	if ((rs = searchkeys_curinc(op,curp,&i,&j)) >= 0) {
	    SKWORD	*wep ;
	    int		wel ;
	    if (i < op->nphrases) {
	        wep = op->kphrases[i].kwords ;
	        wel = op->kphrases[i].nwords ;
	        if (j < wel) {
	            kl = wep[j].kl ;
	            if (rpp != NULL) *rpp = wep[j].kp ;
	            curp->i = i ;
	            curp->j = j ;
	        } else {
	            rs = SR_NOTFOUND ;
		}
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	} /* end if (searchkeys_curinc) */

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (searchkeys_enum) */


/* private subroutines */


static int searchkeys_build(SEARCHKEYS *op,cchar **qsp)
{
	BUILD		bi, *bip = &bi ;
	const int	size = sizeof(BUILDPHRASE) ;
	const int	opts = VECOBJ_OSWAP ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("searchkeys_build: ent\n") ;
#endif

	if ((rs = vecobj_start(&bip->phrases,size,4,opts)) >= 0) {
	    int		i ;

	    for (i = 0 ; qsp[i] != NULL ; i += 1) {
	        rs = searchkeys_buildadd(op,bip,qsp[i]) ;
	        if (rs < 0) break ;
	    } /* end for */

/* finish up */

	    if (rs >= 0) {

#if	CF_BUILDREDUCE
	        rs = searchkeys_buildreduce(op,bip) ;
#endif

	        if (rs >= 0) {
	            rs = searchkeys_buildload(op,bip) ;
	            c = 0 ;
	        }

	    } /* end if */

	    rs1 = searchkeys_buildfins(op,bip) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = vecobj_finish(&bip->phrases) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (phrases) */

#if	CF_DEBUGS
	debugprintf("searchkeys_build: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (searchkeys_build) */


static int searchkeys_buildadd(SEARCHKEYS *op,BUILD *bip,cchar phrase[])
{
	BUILDPHRASE	bpe ;
	int		rs ;
	int		rs1 ;
	int		wc = 0 ;

#if	CF_DEBUGS
	debugprintf("searchkeys_buildadd: p=>%s<\n",phrase) ;
#endif

	if ((rs = buildphrase_start(&bpe)) >= 0) {
	    int		f_match = FALSE ;
	    int		f_buildphrase = TRUE ;
	    cchar	*tp ;
	    cchar	*sp = phrase ;

	    while ((tp = strpbrk(sp," \t,")) != NULL) {
	        if ((tp - sp) > 0) {
	            rs = searchkeys_buildaddword(op,&bpe,sp,(tp - sp)) ;
		}
	        sp = (tp + 1) ;
	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (sp[0] != '\0')) {
	        rs = searchkeys_buildaddword(op,&bpe,sp,-1) ;
	    }

/* conditionally add this phrase to the list */

	    if (rs >= 0) {

	        f_match = FALSE ;
	        rs = buildphrase_count(&bpe) ;
	        wc = rs ;

	        if ((rs >= 0) && (wc == 1)) {
	            rs = searchkeys_buildphrasemat(op,bip,&bpe) ;
	            f_match = (rs > 0) ;
	        }

	        if (rs >= 0) {
	            if ((wc > 0) && (! f_match)) {
	                rs = vecobj_add(&bip->phrases,&bpe) ;
	                f_buildphrase = (rs >= 0) ;
	            } else {
	                f_buildphrase = FALSE ;
	                rs1 = buildphrase_finish(&bpe) ;
	                if (rs >= 0) rs = rs1 ;
	            }
	        } /* end if */

	    } /* end if */

	    if (rs < 0) {
	        if (f_buildphrase) {
	            buildphrase_finish(&bpe) ;
	        }
	    }

	    if (wc > 0)
	        op->nphrases += 1 ;

	} /* end if (buildphrase_start) */

#if	CF_DEBUGS
	debugprintf("searchkeys_buildadd: ret rs=%d wc=%u\n",rs,wc) ;
#endif

	return (rs >= 0) ? wc : rs ;
}
/* end subroutine (searchkeys_buildadd) */


static int searchkeys_buildaddword(SEARCHKEYS *op,BUILDPHRASE *bpp,
		cchar *wp,int wl)
{
	int		rs = SR_OK ;
	int		kl ;
	cchar		*kp ;
	char		keybuf[KEYBUFLEN + 1] ;

	if (wl < 0)
	    wl = strlen(wp) ;

	kp = wp ;
	kl = wl ;
	if (kl > KEYBUFLEN)
	    kl = KEYBUFLEN ;

	if (hasuc(kp,kl)) {
	    strwcpylc(keybuf,kp,kl) ;	/* can't overflow */
	    rs = strpack_store(&op->stores,keybuf,kl,&kp) ;
	} /* end if */

	if (rs >= 0) {
	    rs = buildphrase_add(bpp,kp,kl) ;
	}

	return rs ;
}
/* end subroutine (searchkeys_buildaddword) */


static int searchkeys_buildphrasemat(SEARCHKEYS *op,BUILD *bip,BUILDPHRASE *bpp)
{
	int		rs ;
	int		rs1 ;
	int		f_match = FALSE ;
	cchar		*kp ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = buildphrase_getkey(bpp,0,&kp)) > 0) {
	    BUILDPHRASE	*ptp ;
	    int		kl = rs ;
	    int		i ;
	    for (i = 0 ; vecobj_get(&bip->phrases,i,&ptp) >= 0 ; i += 1) {
	        if (ptp != NULL) {
	            if ((rs1 = buildphrase_havekey(ptp,kp,kl)) > 0) {
	                f_match = TRUE ;
	                break ;
	            }
		}
	    } /* end for */
	} /* end if */

	return (rs >= 0) ? f_match : rs ;
}
/* end subroutine (searchkeys_buildphrasemat) */


static int searchkeys_buildreduce(SEARCHKEYS *op,BUILD *bip)
{
	BUILDPHRASE	*bpp ;
	int		rs = SR_OK ;
	int		i ;
	int		wc ;

#if	CF_DEBUGS
	debugprintf("searchkeys_buildreduce: ent\n") ;
#endif

	for (i = 0 ; vecobj_get(&bip->phrases,i,&bpp) >= 0 ; i += 1) {
	    if (bpp != NULL) {
	        if ((rs = buildphrase_count(bpp)) >= 0) {
	            wc = rs ;
	            if (wc == 1) {
			const int	n = (i+1) ;
	                if ((rs = searchkeys_buildmatone(op,bip,n,bpp)) > 0) {
	                    buildphrase_finish(bpp) ;
	                    vecobj_del(&bip->phrases,i--) ;
	                    if (op->nphrases > 0) op->nphrases -=1 ;
		        }
	            }
	        } /* end if */
	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_ASSERT
	if ((rs = vecobj_count(&bip->phrases)) != op->nphrases) {
	    rs = SR_BADFMT ;
	}
#endif /* CF_ASSERT */

#if	CF_DEBUGS
	debugprintf("searchkeys_buildreduce: ret rs=%d np=%u\n",
	    rs,op->nphrases) ;
#endif

	return (rs >= 0) ? op->nphrases : rs ;
}
/* end subroutine (searchkeys_buildreduce) */


static int searchkeys_buildfins(SEARCHKEYS *op,BUILD *bip)
{
	BUILDPHRASE	*bpp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	for (i = 0 ; vecobj_get(&bip->phrases,i,&bpp) >= 0 ; i += 1) {
	    if (bpp != NULL) {
	        rs1 = buildphrase_finish(bpp) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = vecobj_del(&bip->phrases,i--) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("searchkeys_buildfins: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (searchkeys_buildfins) */


static int searchkeys_buildmatone(SEARCHKEYS *op,BUILD *bip,int si,
		BUILDPHRASE *bpp)
{
	int		rs ;
	int		rs1 ;
	int		f_match = FALSE ;
	cchar		*kp ;

#if	CF_DEBUGS
	debugprintf("searchkeys_buildmatone: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (si < 0)
	    return SR_INVALID ;

	if ((rs = buildphrase_getkey(bpp,0,&kp)) > 0) {
	    BUILDPHRASE	*ptp ;
	    int		kl = rs ;
	    int		i ;
	    for (i = si ; vecobj_get(&bip->phrases,i,&ptp) >= 0 ; i += 1) {
	        if (ptp != NULL) {
	            if ((rs1 = buildphrase_havekey(ptp,kp,kl)) > 0) {
	                f_match = (rs1 > 0) ;
	                break ;
		    }
	        }
	    } /* end for */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("searchkeys_buildmatone: ret rs=%d f=%u\n",rs,f_match) ;
#endif

	return (rs >= 0) ? f_match : rs ;
}
/* end subroutine (searchkeys_buildmatone) */


static int searchkeys_buildload(SEARCHKEYS *op,BUILD *bip)
{
	VECOBJ		*plp = &bip->phrases ;
	int		rs ;
	int		nphrases = 0 ;

#if	CF_DEBUGS
	debugprintf("searchkeys_buildload: ent\n") ;
#endif

	if ((rs = vecobj_count(plp)) >= 0) {
	    BUILDPHRASE	*bpp ;
	    SKWORD	*wep ;
	    int		size ;
	    char	*p ;

	    nphrases = rs ;
	    size = (nphrases + 1) * sizeof(SKPHRASE) ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        int	nwords ;
	        int	pi, pj ;
	        int	wi, wj ;
	        int	kl ;
	        cchar	*kp ;

	        memset(p,0,size) ;
	        op->kphrases = (SKPHRASE *) p ;
	        op->nphrases = nphrases ;

	        pj = 0 ;
	        for (pi = 0 ; vecobj_get(plp,pi,&bpp) >= 0 ; pi += 1) {
	            if (bpp != NULL) {

	            rs = buildphrase_count(bpp) ;
	            if (rs < 0) break ;
	            nwords = rs ;

	            size = (nwords + 1) * sizeof(SKWORD) ;
	            rs = uc_malloc(size,&wep) ;
	            if (rs < 0) break ;

	            op->kphrases[pj].kwords = wep ;
	            op->kphrases[pj].nwords = nwords ;
	            pj += 1 ;

	            wj = 0 ;
	            wi = 0 ;
	            while ((kl = buildphrase_getkey(bpp,wi,&kp)) >= 0) {
	                wep[wj].kp = kp ;
	                wep[wj].kl = kl ;
	                wj += 1 ;
	                wi += 1 ;
	            } /* end while */
	            if (kl != SR_NOTFOUND) rs = kl ;

	            wep[wj].kp = NULL ;
	            wep[wj].kl = 0 ;

		    }
	            if (rs < 0) break ;
	        } /* end for */

	        op->kphrases[pj].kwords = NULL ;
	        op->kphrases[pj].nwords = 0 ;

	        if (rs < 0) {
	            for (pi = 0 ; op->kphrases[pi].kwords != NULL ; pi += 1) {
	                uc_free(op->kphrases[pi].kwords) ;
	                op->kphrases[pi].kwords = NULL ;
	            } /* end for */
	            {
	                uc_free(op->kphrases) ;
	                op->kphrases = NULL ;
	            }
	        }
	    } /* end if (m-a) */
	} /* end if (vecobj_count) */

#if	CF_DEBUGS
	debugprintf("searchkeys_buildload: ret rs=%d np=%u\n",rs,nphrases) ;
#endif

	return (rs >= 0) ? nphrases : rs ;
}
/* end subroutine (searchkeys_buildload) */


static int searchkeys_curinc(SEARCHKEYS *op,SEARCHKEYS_CUR *curp,
		int *ip,int *jp)
{
	int		rs = SR_OK ;

	if (curp->i < 0) {
	    *ip = 0 ;
	    *jp = 0 ;
	} else if (curp->i >= op->nphrases) {
	    rs = SR_NOTFOUND ;
	} else {
	    int	wel ;
	    *ip = curp->i ;
	    *jp = (curp->j < 0) ? 1 : (curp->j + 1) ;
	    wel = op->kphrases[*ip].nwords ;
	    if (*jp >= wel) {
	        *ip = (curp->i + 1) ;
	        *jp = 0 ;
	        if (*ip >= op->nphrases) {
	            rs = SR_NOTFOUND ;
		}
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (searchkeys_curinc) */


#if	CF_DEBUGS && CF_CHECKIT

static int searchkeys_checkit(SEARCHKEYS *op,uint id,cchar *s)
{
	struct searchkeys_kphrase	*kph ;
	int		nkph = op->nphrases ;
	int		i ;
	kph = op->kphrases ;
	debugprintf("searchkeys_checkit: id=%u %s nkph=%u\n",id,s,nkph) ;
	for (i = 0 ; i < nkph ; i += 1) {
	    debugprintf("searchkeys_checkit: id=%u %s i=%u kph{%p}\n",
	        id,s,i,(kph+i)) ;
	}
	return 0 ;
}
/* end subroutine (searchkeys_checkit) */

#endif /* CF_DEBUGS */


static int buildphrase_start(BUILDPHRASE *bpp)
{
	int		rs ;
	int		size ;
	int		opts ;

#if	CF_SAFE
	if (bpp == NULL) return SR_FAULT ;
#endif

	size = sizeof(SKWORD) ;
	opts = (VECOBJ_OCOMPACT | VECOBJ_OSTATIONARY) ;
	rs = vecobj_start(&bpp->words,size,1,opts) ;

	return rs ;
}
/* end subroutine (buildphrase_start) */


static int buildphrase_finish(BUILDPHRASE *bpp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (bpp == NULL) return SR_FAULT ;
#endif

	rs1 = vecobj_finish(&bpp->words) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (buildphrase_finish) */


static int buildphrase_add(BUILDPHRASE *bpp,cchar *kp,int kl)
{
	SKWORD		ke ;
	int		rs ;

#if	CF_SAFE
	if (bpp == NULL) return SR_FAULT ;
#endif

	if (kl < 0)
	    kl = strlen(kp) ;

	ke.kp = kp ;
	ke.kl = kl ;
	rs = vecobj_add(&bpp->words,&ke) ;

	return rs ;
}
/* end subroutine (buildphrase_add) */


static int buildphrase_count(BUILDPHRASE *bpp)
{
	int		rs ;

#if	CF_SAFE
	if (bpp == NULL) return SR_FAULT ;
#endif

	rs = vecobj_count(&bpp->words) ;

	return rs ;
}
/* end subroutine (buildphrase_count) */


static int buildphrase_getkey(BUILDPHRASE *bpp,int i,cchar **rpp)
{
	SKWORD		*kep ;
	int		rs ;
	int		kl = 0 ;

#if	CF_SAFE
	if (bpp == NULL) return SR_FAULT ;
#endif

	if (i < 0)
	    return SR_INVALID ;

	if ((rs = vecobj_get(&bpp->words,i,&kep)) >= 0) {
	    kl = kep->kl ;
	    if (rpp != NULL)
	        *rpp = (cchar *) kep->kp ;
	}

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (buildphrase_getkey) */


static int buildphrase_havekey(BUILDPHRASE *bpp,cchar *kp,int kl)
{
	SKWORD		*kep ;
	int		i ;
	int		f = FALSE ;

#if	CF_SAFE
	if (bpp == NULL) return SR_FAULT ;
#endif

	if (kl < 0)
	    kl = strlen(kp) ;

	for (i = 0 ; vecobj_get(&bpp->words,i,&kep) >= 0 ; i += 1) {
	    if (kep != NULL) {
	        f = (kl == kep->kl) ;
	        f = f && (kep->kp[0] == kp[0]) ;
	        f = f && (strncmp(kep->kp,kp,kl) == 0) ;
	        if (f) break ;
	    }
	} /* end for */

	return f ;
}
/* end subroutine (buildphrase_havekey) */


#if	CF_REGPROC

static int kphrase_process(pep,f_prefix,ki,wp,wl)
SKPHRASE	*pep ;
int		f_prefix ;
int		ki ;
cchar		*wp ;
int		wl ;
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_SAFE
	if (pep == NULL) return SR_FAULT ;
#endif

	if (ki <= pep->nwords) {
	    if (ki < pep->nwords) {
	        SKWORD		*kep ;
	        kep = (pep->kwords + ki) ;
	        if (kep != NULL) {
	            f = (kep->kp[0] == wp[0]) ;
	            if (f) {
	                if (f_prefix) {
	                    int	m = nleadstr(kep->kp,wp,wl) ;
	                    f = ((m > 0) && (kep->kl == m)) ;
	                } else {
	                    f = (wl == kep->kl) ;
	                    f = f && (strncmp(kep->kp,wp,wl) == 0) ;
	                }
	            }
	        } else {
	            rs = SR_FAULT ;
	        }
	    } /* end if (less-than) */
	} else {
	    rs = SR_INVALID ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (kphrase_process) */

#endif /* CF_REGPROC */


static int kphrase_processxw(SKPHRASE *pep,int f_prefix,int ki,XWORDS *xwp)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#ifdef	COMMENT
	if (pep == NULL) return SR_FAULT ;
#endif

#if	CF_DEBUGS && CF_KPHRASE
	debugprintf("kphrase_processxw: ent n=%d ki=%u\n",pep->nwords,ki) ;
#endif

	if (ki <= pep->nwords) {
	    if (ki < pep->nwords) {
	        SKWORD		*kep ;
	        kep = (pep->kwords + ki) ;
	        if (kep != NULL) {
		    int		i = 0 ;
		    int		m ;
		    int		wl ;
		    cchar	*wp ;
	            for (i = 0 ; ((wl = xwords_get(xwp,i,&wp)) > 0) ; i += 1) {
	                f = (kep->kp[0] == wp[0]) ;
#if	CF_SHORTCUT
	                if (! f) break ;
#endif
	                if (f) {
	                    if (f_prefix) {
	                        m = nleadstr(kep->kp,wp,wl) ;
	                        f = ((m > 0) && (kep->kl == m)) ;
	                    } else {
	                        f = (wl == kep->kl) ;
	                        f = f && (strncmp(kep->kp,wp,wl) == 0) ;
	                    }
	                }
	                if (f) break ;
	            } /* end for (checking each "eXtra" word) */
	        } else {
	            rs = SR_FAULT ;
	        }
	    } /* end if (less than) */
	} else {
	    rs = SR_INVALID ;
	}

#if	CF_DEBUGS && CF_KPHRASE
	debugprintf("kphrase_processxw: ret i=%u\n",i) ;
	debugprintf("kphrase_processxw: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (kphrase_processxw) */


