/* tagtrack */

/* track tags in DWB documents */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1987-09-10, David A­D­ Morano
	This code module was originally written.

	= 1998-09-10, David A­D­ Morano
	This module was changed to serve in the REFERM program.

*/

/* Copyright © 1987,1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This code module (object) maintains a citation database. It stores the
        citation keys, and a count for each, that are found within the document
        text.

	No emumeration is required since only lookups by key are needed.


******************************************************************************/


#define	TAGTRACK_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"tagtrack.h"
#include	"findinline.h"


/* local defines */

#define	TAGTRACK_MAGIC		0x31887239
#define	TAGTRACK_DEFENTRIES	20


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfnext(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	tagtrack_addmac(TAGTRACK *,const char *,int) ;
static int	tagtrack_scanescapes(TAGTRACK *,int,uint,const char *,int) ;
static int	tagtrack_search(TAGTRACK *,TAGTRACK_TAG **,const char *,int) ;
static int	tagtrack_addesc(TAGTRACK *,TAGTRACK_TAG *,int,uint,int) ;

int		tagtrack_add(TAGTRACK *,int,uint,int,const char *,int) ;

static int	tag_start(TAGTRACK_TAG *,const char *,int) ;
static int	tag_addnum(TAGTRACK_TAG *,int,int) ;
static int	tag_finish(TAGTRACK_TAG *) ;

static int	entry_load(TAGTRACK_ENT *,TAGTRACK_ESC *) ;

#ifdef	COMMENT
static int	mkcitestr(char *,int) ;
#endif /* COMMENT */

static int	vcmpfor(const void **,const void **) ;


/* local variables */

enum ourmacs {
	ourmac_table,
	ourmac_example,
	ourmac_figure,
	ourmac_equation,
	ourmac_tag,
	ourmac_overlast
} ;

static const char	*ourmacs[] = {
	"TE",
	"EE",
	"FG",
	"EN",
	"TAG",
	NULL
} ;

enum ourescapes {
	ourescape_tag,
	ourescape_under,
	ourescape_overlast
} ;

static const char	*ourescapes[] = {
	"tag",
	"_",
	NULL
} ;


/* exported subroutines */


int tagtrack_start(op)
TAGTRACK	*op ;
{
	const int	n = TAGTRACK_DEFENTRIES ;

	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("tagtrack_start: entered\n") ;
#endif

	memset(op,0,sizeof(TAGTRACK)) ;

	if ((rs = vechand_start(&op->tags,n,0)) >= 0) {
	    const int	size = sizeof(TAGTRACK_ESC) ;
	    if ((rs = vecobj_start(&op->list,size,n,0)) >= 0) {
		op->magic = TAGTRACK_MAGIC ;
	    }
	    if (rs < 0)
		vechand_finish(&op->tags) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("tagtrack_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (tagtrack_start) */


int tagtrack_finish(op)
TAGTRACK	*op ;
{
	TAGTRACK_TAG	*tagp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != TAGTRACK_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = vecobj_finish(&op->list) ;
	if (rs >= 0) rs = rs1 ;

	for (i = 0 ; vechand_get(&op->tags,i,&tagp) >= 0 ; i += 1) {
	    if (tagp == NULL) continue ;
	    rs1 = tag_finish(tagp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(tagp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

	rs1 = vechand_finish(&op->tags) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (tagtrack_finish) */


int tagtrack_scanline(op,fi,loff,lp,ll)
TAGTRACK	*op ;
int		fi ;
uint		loff ;
const char	*lp ;
int		ll ;
{
	int	rs = SR_OK ;
	int	ch = MKCHAR(lp[0]) ;
	int	f_macro = FALSE ;

	if (ch == '.') {
	    const char	*mp ;
	    int		ml ;
	    lp += 1 ;
	    ll -= 1 ;
	    if ((ml = sfnext(lp,ll,&mp)) > 0) {
		const char	*np ;
		int		nl ;
		int		oi ;
		ll -= ((mp+ml)-lp) ;
		lp = (mp+ml) ;
	        if ((oi = matstr(ourmacs,mp,ml)) >= 0) {
#if	CF_DEBUGS
	debugprintf("tagtrack_scanline: ours=%s\n",ourmacs[oi]) ;
#endif
		    switch (oi) {
		    case ourmac_tag:
		        f_macro = TRUE ;
			{
			    const char	*tp ;
			    while ((tp = strnpbrk(lp,ll," ,\t")) != NULL) {
				if ((nl = sfnext(lp,(tp-lp),&np)) > 0) {
			    	    rs = tagtrack_addmac(op,np,nl) ;
				}
				ll -= ((tp+1)-lp) ;
				lp = (tp+1) ;
			    } /* end while */
			    if ((rs >= 0) && (ll > 0)) {
				if ((nl = sfnext(lp,ll,&np)) > 0) {
			    	    rs = tagtrack_addmac(op,np,nl) ;
				}
			    }
			} /* end block */
			break ;
		    case ourmac_table:
		    case ourmac_example:
		    case ourmac_figure:
		    case ourmac_equation:
			op->c[oi] += 1 ;
			op->lc = op->c[oi] ;
			op->ltt = oi ;
			break ;
		    } /* end switch */
		} /* end if (matstr) */
	    } /* end if (get macro name) */
	} else {
	    rs = tagtrack_scanescapes(op,fi,loff,lp,ll) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("tagtrack_scanline: ret rs=%d f_macro=%u\n",rs,f_macro) ;
#endif

	return (rs >= 0) ? f_macro : rs ;
}
/* end subroutine (tagtrack_scanline) */


int tagtrack_adds(op,fi,eoff,elen,kp,kl)
TAGTRACK	*op ;
int		fi ;
uint		eoff ;
int		elen ;
const char	kp[] ;
int		kl ;
{
	int	rs = SR_OK ;
	int	cl ;
	int	c = 0 ;

	const char	*cp ;
	const char	*tp ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != TAGTRACK_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("tagtrack_adds: fi=%u eoff=%u elen=%d k=>%t<\n",
		fi,eoff,elen,kp,kl) ;
#endif


	if (kl < 0)
	    kl = strlen(kp) ;

	while ((tp = strnchr(kp,kl,',')) != NULL) {
	    if ((cl = sfshrink(kp,(tp - kp),&cp)) > 0) {
		c += 1 ;
	        rs = tagtrack_add(op,fi,eoff,elen,cp,cl) ;
	    }
	    kl -= ((tp + 1) - kp) ;
	    kp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (kl > 0)) {
	    if ((cl = sfshrink(kp,kl,&cp)) > 0) {
		c += 1 ;
	        rs = tagtrack_add(op,fi,eoff,elen,cp,cl) ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (tagtrack_adds) */


/* load a string parameter into the DB */
int tagtrack_add(op,fi,eoff,elen,np,nl)
TAGTRACK	*op ;
int		fi ;
uint		eoff ;
int		elen ;
const char	np[] ;
int		nl ;
{
	TAGTRACK_TAG	*tagp ;

	int	rs ;


	if (op == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;

	if (op->magic != TAGTRACK_MAGIC) return SR_NOTOPEN ;
	if (fi < 0) return SR_INVALID ;
	if (np[0] == '\0') return SR_INVALID ;

	if (nl < 0)
	    nl = strlen(np) ;

#if	CF_DEBUGS
	debugprintf("tagtrack_add: eoff=%u elen=%d\n",eoff,elen) ;
	debugprintf("tagtrack_add: n=%t\n",np,nl) ;
#endif

	if ((rs = tagtrack_search(op,&tagp,np,nl)) >= 0) {
	    rs = tagtrack_addesc(op,tagp,fi,eoff,elen) ;
	} else if (rs == SR_NOTFOUND) {
	    const int	size = sizeof(TAGTRACK_TAG) ;
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        tagp = p ;
	        if ((rs = tag_start(tagp,np,nl)) >= 0) {
	    	    if ((rs = tagtrack_addesc(op,tagp,fi,eoff,elen)) >= 0) {
		        rs = vechand_add(&op->tags,tagp) ;
		    }
		    if (rs < 0)
			tag_finish(tagp) ;
		} /* end if (tag-start) */
		if (rs < 0)
		    uc_free(tagp) ;
	    } /* end if (memory-allocated) */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("tagtrack_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (tagtrack_add) */


int tagtrack_curbegin(op,curp)
TAGTRACK	*op ;
TAGTRACK_CUR	*curp ;
{


#if	CF_DEBUGS
	debugprintf("tagtrack_curbegin: entered \n") ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != TAGTRACK_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (tagtrack_curbegin) */


int tagtrack_curend(op,curp)
TAGTRACK	*op ;
TAGTRACK_CUR	*curp ;
{


#if	CF_DEBUGS
	debugprintf("tagtrack_curend: entered \n") ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != TAGTRACK_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (tagtrack_curend) */


int tagtrack_enum(op,curp,ep)
TAGTRACK	*op ;
TAGTRACK_CUR	*curp ;
TAGTRACK_ENT	*ep ;
{
	TAGTRACK_ESC	*offp ;

	int	rs ;
	int	i ;


#if	CF_DEBUGS
	debugprintf("tagtrack_enum: entered \n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

	if (op->magic != TAGTRACK_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

/* do the lookup */

	while ((rs = vecobj_get(&op->list,i,&offp)) >= 0) {
	    if (offp != NULL) break ;
	    i += 1 ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("tagtrack_enum: vecobj_get() rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (offp != NULL)) {
	    if (ep != NULL)
	        rs = entry_load(ep,offp) ;
	    curp->i = i ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (tagtrack_enum) */


int tagtrack_audit(op)
TAGTRACK	*op ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != TAGTRACK_MAGIC)
	    return SR_NOTOPEN ;

	rs = vechand_audit(&op->tags) ;

#if	CF_DEBUGS
	debugprintf("tagtrack_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (tagtrack_audit) */


/* private subroutines */


static int tagtrack_addmac(TAGTRACK *op,const char *np,int nl)
{
	TAGTRACK_TAG	*tagp ;

	int	rs ;

#if	CF_DEBUGS
	debugprintf("tagtrack_addmac: n=%t\n",np,nl) ;
	debugprintf("tagtrack_addmac: ltt=%d lc=%d\n",op->ltt,op->lc) ;
#endif

	if ((rs = tagtrack_search(op,&tagp,np,nl)) >= 0) {
#if	CF_DEBUGS
	debugprintf("tagtrack_addmac: found already\n") ;
#endif
	    rs = tag_addnum(tagp,op->ltt,op->lc) ;
	} else if (rs == SR_NOTFOUND) {
	    const int	size = sizeof(TAGTRACK_TAG) ;
	    void	*p ;
#if	CF_DEBUGS
	debugprintf("tagtrack_addmac: new tag\n") ;
#endif
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        tagp = p ;
	        if ((rs = tag_start(tagp,np,nl)) >= 0) {
		    if ((rs = tag_addnum(tagp,op->ltt,op->lc)) >= 0) {
		        rs = vechand_add(&op->tags,tagp) ;
#if	CF_DEBUGS
	debugprintf("tagtrack_addmac: vechand_add() rs=%d\n",rs) ;
#endif
		    }
		    if (rs < 0)
			tag_finish(tagp) ;
		} /* end if (tag-start) */
		if (rs < 0)
		    uc_free(tagp) ;
	    } /* end if (memory-allocated) */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("tagtrack_addmac: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (tagtrack_addmac) */


static int tagtrack_scanescapes(op,fi,loff,lp,ll)
TAGTRACK	*op ;
int		fi ;
uint		loff ;
const char	*lp ;
int		ll ;
{
	FINDINLINE	esc ;

	uint	eoff ;

	int	rs = SR_OK ;
	int	sl ;
	int	ei ;
	int	c = 0 ;
	const char	*linestart = lp ;

#if	CF_DEBUGS
	debugprintf("tagtrack_scanescapes: fi=%u loff=%u\n",fi,loff) ;
	debugprintf("tagtrack_scanescapes: l=>%t<\n",lp,ll) ;
#endif

	if (ll < 0) ll = strlen(lp) ;

	while ((sl = findinline(&esc,lp,ll)) > 0) {

#if	CF_DEBUGS
	debugprintf("tagtrack_scanescapes: found sl=%d\n",sl) ;
#endif

	    if ((ei = matstr(ourescapes,esc.kp,esc.kl)) >= 0) {
		c += 1 ;
		switch (ei) {
		case ourescape_tag:
		case ourescape_under:
		    eoff = (loff + (esc.sp-linestart)) ;
		    rs = tagtrack_adds(op,fi,eoff,sl,esc.vp,esc.vl) ;
		    break ;
		} /* end switch */
	    } /* end if (matstr) */
	    ll -= ((esc.sp+sl)-lp) ;
	    lp = (esc.sp+sl) ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("tagtrack_scanescapes: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (tagtrack_scanescapes) */


static int tagtrack_search(op,tagpp,np,nl)
TAGTRACK	*op ;
TAGTRACK_TAG	**tagpp ;
const char	*np ;
int		nl ;
{
	TAGTRACK_TAG	te ;
	NULSTR		tn ;
	int		rs ;
	const char	*name = NULL ;

	if ((rs = nulstr_start(&tn,np,nl,&name)) >= 0) {

	   te.name = name ;
	   rs = vechand_search(&op->tags,&te,vcmpfor,tagpp) ;

	   nulstr_finish(&tn) ;
	} /* end if (nulstr) */

	return rs ;
}
/* end subroutine (tagtrack_search) */


static int tagtrack_addesc(op,tagp,fi,eoff,elen)
TAGTRACK	*op ;
TAGTRACK_TAG	*tagp ;
int		fi ;
uint		eoff ;
int		elen ;
{
	TAGTRACK_ESC	esc ;

	int	rs ;

#if	CF_DEBUGS
	debugprintf("tagtrack_addesc: store eoff=%u\n",eoff) ;
#endif

	    memset(&esc,0,sizeof(TAGTRACK_ESC)) ;
	    esc.tagp = tagp ;
	    esc.fi = fi ;
	    esc.eoff = eoff ;
	    esc.elen = elen ;

	    rs = vecobj_add(&op->list,&esc) ;

#if	CF_DEBUGS
	debugprintf("tagtrack_addesc: vecobj_add() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (tagtrack_addesc) */


static int tag_start(tagp,kp,kl)
TAGTRACK_TAG	*tagp ;
const char	*kp ;
int		kl ;
{
	int	rs ;

	const char	*cp ;

	if (tagp == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	memset(tagp,0,sizeof(TAGTRACK_TAG)) ;
	tagp->tagtype = -1 ;

	if ((rs = uc_mallocstrw(kp,kl,&cp)) >= 0) {
	    tagp->name = cp ;
	}

	return rs ;
}
/* end subroutine (tag_start) */


static int tag_finish(sp)
TAGTRACK_TAG	*sp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (sp == NULL)
	    return SR_FAULT ;

	if (sp->name != NULL) {
	    rs1 = uc_free(sp->name) ;
	    if (rs >= 0) rs = rs1 ;
	    sp->name = NULL ;
	}

	return rs ;
}
/* end subroutine (tag_finish) */


static int tag_addnum(TAGTRACK_TAG *tagp,int ltt,int lc)
{
	int	rs = SR_OK ;
	if (tagp->c <= 0) {
	    tagp->tagtype = ltt ;
	    tagp->c = lc ;
	} else
	    rs = SR_INVALID ;
	return rs ;
}
/* end subroutine (tag_addnum) */


static int entry_load(ep,offp)
TAGTRACK_ENT	*ep ;
TAGTRACK_ESC	*offp ;
{
	TAGTRACK_TAG	*tagp = offp->tagp ;

	int	rs = SR_OK ;

	ep->fi = offp->fi ;
	ep->eoff = offp->eoff ;
	ep->elen = offp->elen ;
	ep->v = tagp->c ;

	return rs ;
}
/* end subroutine (entry_load) */


static int vcmpfor(v1pp,v2pp)
const void	**v1pp, **v2pp ;
{
	TAGTRACK_TAG	**e1pp = (TAGTRACK_TAG **) v1pp ;
	TAGTRACK_TAG	**e2pp = (TAGTRACK_TAG **) v2pp ;
	int	rc ;
	if ((*e1pp == NULL) && (*e2pp == NULL)) return 0 ;
	if (*e1pp == NULL) return 1 ;
	if (*e2pp == NULL) return -1 ;
	rc = ((*e1pp)->name[0] - (*e2pp)->name[0]) ;
	if (rc == 0)
	    rc = strcmp((*e1pp)->name,(*e2pp)->name) ;
	return rc ;
}
/* end subroutine (vcmpfor) */


