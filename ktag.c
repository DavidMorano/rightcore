/* ktag */

/* tag accummulator object */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_EMPTYTERM	1		/* empty line terminates entry */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object provides management for the accumulation of text
	words in order to form tags for text indexing.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<txtindexmk.h>
#include	<eigendb.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<field.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"xwords.h"
#include	"naturalwords.h"
#include	"ktag.h"


/* local defines */

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	NATURALWORDLEN
#endif


/* external subroutines */

extern int	snwcpylc(char *,int,const char *,int) ;
extern int	sfword(const char *,int,const char **) ;
extern int	field_word(FIELD *,const uchar *,const char **) ;
extern int	hasuc(const char *,int) ;

extern char	*strwcpylc(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	vesrch(const void *,const void *) ;


/* exported subroutines */


int ktag_start(KTAG *kop,KTAG_PARAMS *kap,uint soff,cchar *lp,int ll)
{
	const int	size = sizeof(KTAG_KEY) ;
	const int	vopts = VECOBJ_OCOMPACT ;
	int		rs ;

	memset(kop,0,sizeof(KTAG)) ;
	kop->kap = kap ;
	kop->recoff = soff ;

	if ((rs = vecobj_start(&kop->keys,size,0,vopts)) >= 0) {

#ifdef	COMMENT /* seldom used -- so initialize as needed */
	    rs = vecstr_start(&kop->store,5,0) ;
	    kop->f_store = (rs >= 0) ;
#endif /* COMMENT */

	    if (rs >= 0) {
	        rs = ktag_procline(kop,lp,ll) ;
	        if (rs < 0) {
	            if (kop->f_store) {
	                vecstr_finish(&kop->store) ;
	            }
	        }
	    }

	    if (rs < 0)
	        vecobj_finish(&kop->keys) ;
	} /* end if (vecobj_start) */

	return rs ;
}
/* end subroutine (ktag_start) */


int ktag_finish(KTAG *kop)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (kop->tkeys != NULL) {
	    rs1 = uc_free(kop->tkeys) ;
	    if (rs >= 0) rs = rs1 ;
	    kop->tkeys = NULL ;
	}

	if (kop->f_store) {
	    rs1 = vecstr_finish(&kop->store) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = vecobj_finish(&kop->keys) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (ktag_finish) */


int ktag_add(KTAG *kop,cchar *lp,int ll)
{
	int		rs ;

	rs = ktag_procline(kop,lp,ll) ;

	return rs ;
}
/* end subroutine (ktag_add) */


int ktag_mktag(KTAG *kop,uint endoff,TXTINDEXMK_TAG *tagp)
{
	KTAG_KEY	**va ;
	int		rs ;

	kop->reclen = (endoff - kop->recoff) ;
	memset(tagp,0,sizeof(TXTINDEXMK_TAG)) ;

	tagp->fname = kop->fname ;	/* it is NULL! (deletion candidate) */
	tagp->recoff = kop->recoff ;
	tagp->reclen = kop->reclen ;
	if ((rs = vecobj_getvec(&kop->keys,&va)) >= 0) {
	    TXTINDEXMK_KEY	*kea = NULL ;
	    int			size ;
	    int			i ;
	    tagp->nkeys = rs ;

#if	CF_DEBUGS && 0
	    {
	        KTAG_KEY	*ep ;
	        for (i = 0 ; vecobj_get(&kop->keys,i,&ep) >= 0 ; i += 1) {
	            debugprintf("ktag_mktag: key=>%t<\n",ep->kp,ep->kl) ;
	        }
	    }
#endif /* CF_DEBUGS */

	    size = tagp->nkeys * sizeof(TXTINDEXMK_KEY) ;
	    if ((rs = uc_malloc(size,&kea)) >= 0) {

	        kop->tkeys = kea ;		/* kea: save for us */
	        for (i = 0 ; i < tagp->nkeys ; i += 1) {
	            kea[i] = *(va[i]) ;
	        } /* end for */

	        tagp->keys = kea ;		/* kea: store in the tag */

#if	CF_DEBUGS && 0
	        for (i = 0 ; i < tagp->nkeys ; i += 1) {
	            debugprintf("ktag_mktag: key=>%t<\n",
	                tagp->keys[i].kp, tagp->keys[i].kl) ;
	        }
#endif /* CF_DEBUGS */

	    } /* end if (m-a) */

	} /* end if (vecobj_getvec) */

	return rs ;
}
/* end subroutine (ktag_mktag) */


int ktag_procline(KTAG *kop,cchar *lp,int ll)
{
	FIELD		fsb ;
	int		rs ;

	if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	    KTAG_PARAMS	*kap = kop->kap ;
	    XWORDS	w ;
	    int		i ;
	    int		fl, sl, cl ;
	    const char	*fp, *sp, *cp ;

	    while ((fl = field_word(&fsb,kap->wterms,&fp)) >= 0) {

/* remove possible apostrophe (single-quote) from leading edge */

	        if (fl && (fp[0] == CH_SQUOTE)) {
	            fp += 1 ;
	            fl -= 1 ;
	        }

	        if (fl < kap->minwlen) continue ;

/* remove possible trailing apostrophe (single-quote) */

	        sl = sfword(fp,fl,&sp) ;

/* remove short words */

	        if (sl < kap->minwlen) continue ;

/* be liberal and fabricate extra keys for matching purposes */

	        if ((sl > 0) && ((rs = xwords_start(&w,sp,sl)) >= 0)) {

	            i = 0 ;
	            while ((rs >= 0) &&
	                ((cl = xwords_get(&w,i++,&cp)) > 0)) {

	                if (cl >= kap->minwlen) {
	                    rs = ktag_procword(kop,cp,cl) ;
			}

	            } /* end while */

	            xwords_finish(&w) ;
	        } /* end if (xwords) */

	        if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return rs ;
}
/* end subroutine (ktag_procline) */


int ktag_procword(KTAG *kop,cchar *cp,int cl)
{
	KTAG_KEY	key ;
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		f_needstore = FALSE ;
	const char	*lcp ;
	char		keybuf[KEYBUFLEN + 1] ;

#if	CF_DEBUGS && 0
	debugprintf("ktag_procword: k=%t\n",cp,cl) ;
#endif

	if (cl > KEYBUFLEN)
	    cl = KEYBUFLEN ;

	if (hasuc(cp,cl)) {
	    f_needstore = TRUE ;
	    strwcpylc(keybuf,cp,cl) ;
	    cp = keybuf ;
	}

/* note that the TXTINDEX object filters out eigen keys also */

	key.kp = (char *) cp ;
	key.kl = cl ;
	if ((rs = vecobj_search(&kop->keys,&key,vesrch,NULL)) == nrs) {
	    rs = SR_OK ;

	    if (f_needstore) {
	        rs = ktag_storelc(kop,&lcp,cp,cl) ;
	        cl = rs ;
	        cp = lcp ;
	    }

	    if (rs >= 0) {
	        key.kp = (char *) cp ;
	        key.kl = cl ;
	        rs = vecobj_add(&kop->keys,&key) ;
	    }

	} /* end if (unique key) */

	return rs ;
}
/* end subroutine (ktag_procword) */


int ktag_storelc(KTAG *kop,cchar **rpp,cchar *cp,int cl)
{
	int		rs = SR_OK ;

	if (! kop->f_store) {
	    rs = vecstr_start(&kop->store,5,0) ;
	    kop->f_store = (rs >= 0) ;
	}

	if (rs >= 0) {
	    if (kop->f_store) {
	        if ((rs = vecstr_add(&kop->store,cp,cl)) >= 0) {
	            rs = vecstr_get(&kop->store,rs,rpp) ;
	        }
	    } else
	        rs = SR_NOANODE ;
	} /* end if (ok) */

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (ktag_storelc) */


/* private subroutines */


/* find if two entries match (we don't need a "comparison") */
static int vesrch(const void *v1p,const void *v2p)
{
	KTAG_KEY	**e1pp = (KTAG_KEY **) v1p ;
	KTAG_KEY	**e2pp = (KTAG_KEY **) v2p ;
	int		rc = 0 ;

	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            if ((rc = ((*e1pp)->kl - (*e2pp)->kl)) == 0) {
	                if ((rc = ((*e1pp)->kp[0] - (*e2pp)->kp[0])) == 0) {
	                    rc = memcmp((*e1pp)->kp,(*e2pp)->kp,(*e1pp)->kl) ;
	                }
	            }
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}

	return rc ;
}
/* end subroutine (vesrch) */


