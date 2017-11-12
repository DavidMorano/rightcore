/* rtags */

/* store result tags from a query */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		1


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module stores and manages result tags (see MKKEY, MKINV,
	and MKQUERY) from a query.  This object really exists to allow for easy
	sorting of the tags after all are acquired from the query.


*******************************************************************************/


#define	RTAGS_MASTER		0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<vecobj.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"rtags.h"


/* local defines */

#define	RTAGS_FNAME	struct rtags_fname
#define	RTAGS_TE	struct rtags_te


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;


/* external variables */


/* local structures */

struct rtags_fname {
	const char	*name ;
	int		fi ;
} ;


/* forward references */

static int	fname_start(struct rtags_fname *,const char *,int) ;
static int	fname_finish(struct rtags_fname *) ;

static int	tag_start(struct rtags_te *,int,int,int) ;
static int	tag_finish(struct rtags_te *) ;

static int	cmpdef() ;


/* local variables */


/* exported subroutines */


int rtags_start(RTAGS *op,int n)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (n < 10)
	    n = 10 ;

	memset(op,0,sizeof(RTAGS)) ;

	if ((rs = hdb_start(&op->fni,n,1,NULL,NULL)) >= 0) {
	    int	opts = VECOBJ_OSTATIONARY ;
	    int	size = sizeof(struct rtags_fname) ;
	    if ((rs = vecobj_start(&op->fnames,size,n,opts)) >= 0) {
	        size = sizeof(struct rtags_te) ;
	        opts = 0 ;
	        if ((rs = vecobj_start(&op->tags,size,n,opts)) >= 0) {
	            if ((rs = ptm_create(&op->m,NULL)) >= 0) {
	                op->magic = RTAGS_MAGIC ;
	            }
	            if (rs < 0)
	                vecobj_finish(&op->tags) ;
	        }
	        if (rs < 0)
	            vecobj_finish(&op->fnames) ;
	    }
	    if (rs < 0)
	        hdb_finish(&op->fni) ;
	}

	return rs ;
}
/* end subroutine (rtags_start) */


int rtags_finish(RTAGS *op)
{
	struct rtags_fname	*fep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != RTAGS_MAGIC) return SR_NOTOPEN ;
#endif

	ptm_destroy(&op->m) ;

/* free up all files */

	for (i = 0 ; vecobj_get(&op->fnames,i,&fep) >= 0 ; i += 1) {
	    if (fep != NULL) {
	        rs1 = fname_finish(fep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

/* free up all tags */

	rs1 = vecobj_finish(&op->tags) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->fnames) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = hdb_finish(&op->fni) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (rtags_finish) */


/* add a tag to the DB */
int rtags_add(RTAGS *op,RTAGS_TAG *tip)
{
	RTAGS_TE	te ;
	RTAGS_FNAME	fe, *fep ;
	HDB_DATUM	key, value ;
	int		rs ;
	int		fi ;

	if (op == NULL) return SR_FAULT ;
	if (tip == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != RTAGS_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    const int	nrs = SR_NOTFOUND ;

#if	CF_DEBUGS
	    debugprintf("rtags_add: fname=%s recoff=%u\n",
	        tip->fname,tip->recoff) ;
#endif

	    key.buf = tip->fname ;
	    key.len = strlen(tip->fname) ;

	    value.buf = NULL ;
	    value.len = 0 ;

/* is the filename already present? */

#if	CF_DEBUGS
	    debugprintf("rtags_add: fetch check\n") ;
#endif

	    if ((rs = hdb_fetch(&op->fni,key,NULL,&value)) == nrs) {

#if	CF_DEBUGS
	        debugprintf("rtags_add: allocating new entry\n") ;
#endif

	        if ((rs = fname_start(&fe,tip->fname,op->nfiles)) >= 0) {

	            if ((rs = vecobj_add(&op->fnames,&fe)) >= 0) {
	                fi = rs ;

	                vecobj_get(&op->fnames,fi,&fep) ;

	                key.buf = fep->name ;
	                key.len = strlen(fep->name) ;

	                value.buf = fep ;
	                value.len = sizeof(struct rtags_fname) ;

	                rs = hdb_store(&op->fni,key,value) ;

#if	CF_DEBUGS
	                debugprintf("rtags_add: hdb_store() rs=%d\n",rs) ;
#endif

	                if (rs < 0)
	                    vecobj_del(&op->fnames,fi) ;

	            } else {
	                fname_finish(&fe) ;
		    }

	        } /* end if */

	        if (rs >= 0)
	            op->nfiles += 1 ;

	    } else if (rs >= 0) {

	        fep = (struct rtags_fname *) value.buf ;
	        fi = fep->fi ;

	    } /* end if */

	    if (rs >= 0) {

#if	CF_DEBUGS
	        debugprintf("rtags_add: fi=%u\n",fi) ;
#endif

	        if ((rs = tag_start(&te,fi,tip->recoff,tip->reclen)) >= 0) {
	            rs = vecobj_add(&op->tags,&te) ;
	            if (rs < 0)
	                tag_finish(&te) ;
	        }

	    } /* end if */

	    ptm_unlock(&op->m) ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (rtags_add) */


int rtags_sort(RTAGS *op,int (*cmpfunc)())
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != RTAGS_MAGIC) return SR_NOTOPEN ;
#endif

	if (cmpfunc == NULL)
	    cmpfunc = cmpdef ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    rs = vecobj_sort(&op->tags,cmpfunc) ;
	    ptm_unlock(&op->m) ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (rtags_sort) */


int rtags_curbegin(RTAGS *op,RTAGS_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != RTAGS_MAGIC) return SR_NOTOPEN ;
#endif

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (rtags_curbegin) */


int rtags_curend(RTAGS *op,RTAGS_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != RTAGS_MAGIC) return SR_NOTOPEN ;
#endif

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (rtags_curend) */


int rtags_curdump(RTAGS *op,RTAGS_CUR *curp)
{
	RTAGS_FNAME	*fep ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != RTAGS_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    int		i ;

/* free up all files */

	    for (i = 0 ; vecobj_get(&op->fnames,i,&fep) >= 0 ; i += 1) {
	        if (fep != NULL) {
	            rs1 = fname_finish(fep) ;
	    	    if (rs >= 0) rs = rs1 ;
	        }
	    } /* end for */

	    rs1 = vecobj_delall(&op->tags) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = vecobj_delall(&op->fnames) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = hdb_delall(&op->fni) ;
	    if (rs >= 0) rs = rs1 ;

	    curp->i = -1 ;

	    ptm_unlock(&op->m) ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (rtags_curdump) */


/* enumerate */
int rtags_enum(RTAGS *op,RTAGS_CUR *curp,RTAGS_TAG *tip)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (tip == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != RTAGS_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    RTAGS_TE	*tep ;
	    int		i ;

	    i = (curp->i < 0) ? 0 : (curp->i + 1) ;

#if	CF_DEBUGS
	    debugprintf("rtags_enum: i=%u\n",i) ;
#endif

	    while ((rs = vecobj_get(&op->tags,i,&tep)) >= 0) {
		if (tep != NULL) break ;

#if	CF_DEBUGS
	        debugprintf("rtags_enum: get-loop i=%u\n",i) ;
#endif

	        i += 1 ;
	    } /* end while */

	    if (rs >= 0) {
	        RTAGS_FNAME	*fep ;

#if	CF_DEBUGS
	        debugprintf("rtags_enum: fi=%u\n",tep->fi) ;
	        debugprintf("rtags_enum: recoff=%u\n",tep->recoff) ;
#endif

	        tip->recoff = tep->recoff ;
	        tip->reclen = tep->reclen ;
	        if ((rs = vecobj_get(&op->fnames,tep->fi,&fep)) >= 0) {
	            if (fep != NULL) {
	                rs = mkpath1(tip->fname,fep->name) ;
	                if (rs >= 0) curp->i = i ;
	            } else {
	                rs = SR_NOANODE ;
		    }
	        }
	    }

	    ptm_unlock(&op->m) ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (rtags_enum) */


int rtags_count(RTAGS *op)
{
	int		rs ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != RTAGS_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&op->m)) >= 0) {

	    rs = vecobj_count(&op->tags) ;
	    c = rs ;

	    ptm_unlock(&op->m) ;
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (rtags_count) */


/* private subroutines */


static int fname_start(RTAGS_FNAME *fep,cchar *fname,int fi)
{
	int		rs ;

#ifdef	OPTIONAL
	if (fep == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;
#endif /* OPTIONAL */

	fep->fi = fi ;
	rs = uc_mallocstrw(fname,-1,&fep->name) ;

	return rs ;
}
/* end subroutine (fname_start) */


static int fname_finish(RTAGS_FNAME *fep)
{
	int		rs = SR_OK ;
	int		rs1 ;

#ifdef	OPTIONAL
	if (fep == NULL) return SR_FAULT ;
#endif

	if (fep->name != NULL) {
	    rs1 = uc_free(fep->name) ;
	    if (rs >= 0) rs = rs1 ;
	    fep->name = NULL ;
	}

	return rs ;
}
/* end subroutine (fname_finish) */


static int tag_start(RTAGS_TE *tep,int fi,int recoff,int reclen)
{

	tep->fi = fi ;
	tep->recoff = recoff ;
	tep->reclen = reclen ;
	return SR_OK ;
}
/* end subroutine (tag_start) */


static int tag_finish(RTAGS_TE *tep)
{

	tep->fi = -1 ;
	return SR_OK ;
}
/* end subroutine (tag_finish) */


static int cmpdef(RTAGS_TE **e1pp,RTAGS_TE **e2pp)
{
	int		rc  = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            if ((rc = ((*e1pp)->fi - (*e2pp)->fi)) == 0) {
	                rc = (*e1pp)->recoff - (*e2pp)->recoff ;
	            }
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpdef) */


