/* fifoelem */

/* FIFO string operations */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history :

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages a FIFO of fixed sized entries
	and of fixed sized depth.


*******************************************************************************/


#define	FIFOELEM_MASTER		0 /* do not claim */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"fifoelem.h"


/* local defines */


/* type defs */

typedef int	(*cmpfun_t)(const void **,const void **) ;


/* external subroutines */


/* forward referecens */

int		fifoelem_fetch(FIFOELEM *,FIFOELEM_CURSOR *,FIFOELEM_ENT **) ;

static int	entry_start(FIFOELEM_ENT *,void *,int) ;
static int	entry_finish(FIFOELEM_ENT *) ;
static int	defaultcmp(const void *,const void *) ;


/* exported subroutines */


int fifoelem_start(fifoelem *fsp)
{
	fsp->head = fsp->tail = NULL ;
	fsp->n = 0 ;
	fsp->magic = FIFOELEM_MAGIC ;
	return SR_OK ;
}
/* end subroutine (fifoelem_start) */


/* free up the entire vector string data structure object */
int fifoelem_finish(fifoelem *fsp)
{
	int		rs = SR_OK ;

	if (fsp == NULL) return SR_FAULT ;

	if (fsp->magic != FIFOELEM_MAGIC) return SR_NOTOPEN ;

	while ((rs = fifoelem_del(fsp,NULL)) >= 0) ;
	if (rs == SR_NOTFOUND) rs = SR_OK ;

	fsp->magic = 0 ;
	return rs ;
}
/* end subroutine (fifoelem_finish) */


/* add a string (to the tail) */
int fifoelem_add(fifoelem *fsp,void *s,int slen)
{
	const int	esize = sizeof(FIFOELEM_ENT) ;
	int		rs ;
	void		*np ;

#if	CF_DEBUGS
	eprintf("fifoelem_add: ent\n") ;
#endif

	if (fsp == NULL) return SR_FAULT ;
	if (s == NULL) return SR_FAULT ;

	if (fsp->magic != FIFOELEM_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	eprintf("fifoelem: s=%s len=%d\n",s,slen) ;
#endif

	if ((rs = uc_malloc(esize,&np)) >= 0) {
	    FIFOELEM_ENT	*ep = (FIFOELEM_ENT *) np ;
	    if ((rs = entry_start(ep,s,slen)) >= 0) {
	        if (fsp->head == NULL) {
	            fsp->head = ep ;
	            fsp->tail = ep ;
	            ep->previous = ep->next = NULL ;
	        } else {
	            ep->previous = fsp->tail ;
	            (fsp->tail)->next = ep ;
	            fsp->tail = ep ;
	        }
	        fsp->n += 1 ;
	        if (rs < 0)
		    uc_free(ep) ;
	    } /* end if (entry_start) */
	} /* end if (m-a) */

	return (rs >= 0) ? fsp->n : rs ;
}
/* end subroutine (fifoelem_add) */


/* remove a string (from the head) */
int fifoelem_remove(fifoelem *fsp,void *ebuf,int elen)
{
	FIFOELEM_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		slen ;

#if	CF_DEBUGS
	eprintf("fifoelem_remove: ent\n") ;
#endif

	if (fsp == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;

	if (fsp->magic != FIFOELEM_MAGIC) return SR_NOTOPEN ;

/* can we give the call an entry ? */

	if (fsp->head == NULL)
	    return SR_NOTFOUND ;

	ep = fsp->head ;
	if ((elen >= 0) && (ep->slen > elen))
	    return SR_TOOBIG ;

	memcpy(ebuf,ep->s,ep->slen) ;

	slen = ep->slen ;
	fsp->head = ep->next ;
	if (fsp->head == NULL) {
	    fsp->tail = NULL ;
	} else {
	    (fsp->head)->previous = NULL ;
	}

	rs1 = entry_finish(ep) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = uc_free(ep) ;
	if (rs >= 0) rs = rs1 ;

	fsp->n -= 1 ;
	return (rs >= 0) ? slen : rs ;
}
/* end subroutine (fifoelem_remove) */


/* return a pointer to an entry that is under the cursor (or at the head) */
int fifoelem_fetch(fifoelem *fsp,FIFOELEM_CURSOR *cp,FIFOELEM_ENT **epp)
{
	FIFOELEM_ENT	*ep ;

	if (fsp == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (fsp->magic != FIFOELEM_MAGIC) return SR_NOTOPEN ;

/* OK, do the deed */

	if ((cp == NULL) || (cp->current == NULL)) {
	    ep = fsp->head ;
	} else {
	    ep = (cp->current)->next ;
	}

	if (cp != NULL)
	    cp->current = ep ;

	if (epp != NULL)
	    *epp = ep ;

	return ((ep != NULL) ? ep->slen : SR_NOTFOUND) ;
}
/* end subroutine (fifoelem_fetch) */


/* delete a string that is under the cursor (or at the head) */
int fifoelem_del(fifoelem *fsp,FIFOELEM_CURSOR *cp)
{
	FIFOELEM_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (fsp == NULL) return SR_FAULT ;

	if (fsp->magic != FIFOELEM_MAGIC) return SR_NOTOPEN ;

/* OK, do the deed */

	if ((cp == NULL) || (cp->current == NULL)) {
	    ep = fsp->head ;
	} else {
	    ep = cp->current ;
	}

	if (ep == NULL)
	    return SR_NOTFOUND ;

	if (cp != NULL) {

	    if (ep->previous == NULL) {
	        fsp->head = ep->next ;
	    } else {
	        (ep->previous)->next = ep->next ;
	    }

	    if (ep->next == NULL) {
	        fsp->tail = ep->previous ;
	    } else {
	        (ep->next)->previous = ep->previous ;
	    }

	    cp->current = ep->previous ;

	} else {

	    fsp->head = ep->next ;
	    if (fsp->head == NULL) {
	        fsp->tail = NULL ;
	    } else {
	        (fsp->head)->previous = NULL ;
	    }

	} /* end if */

	rs1 = entry_finish(ep) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = uc_free(ep) ;
	if (rs >= 0) rs = rs1 ;

	fsp->n -= 1 ;
	return (rs >= 0) ? fsp->n : rs ;
}
/* end subroutine (fifoelem_del) */


/* return the count of the number of items in this list */
int fifoelem_count(fifoelem *fsp)
{

	if (fsp == NULL) return SR_FAULT ;

	if (fsp->magic != FIFOELEM_MAGIC) return SR_NOTOPEN ;

	return fsp->n ;
}
/* end subroutine (fifoelem_count) */


/* search for a string in the FIFO string object */
int fifoelem_finder(fifoelem *fsp,void *s,cmpfun_t cmpfunc,void **rpp)
{
	fifoelem_cur	cur ;
	int		rs ;
	int		rl = 0 ;

	if (fsp == NULL) return SR_FAULT ;

	if (fsp->magic != FIFOELEM_MAGIC) return SR_NOTOPEN ;

	if (cmpfunc == NULL)
	    cmpfunc = (cmpfun_t) defaultcmp ;

	if ((rs = fifoelem_curbegin(fsp,&cur)) >= 0) {
	    FIFOELEM_ENT	*ep ;
	    while ((rs = fifoelem_fetch(fsp,&cur,&ep)) >= 0) {
	        if ((*cmpfunc)(s,(*rpp)) == 0) {
		    rl = ep->slen ;
		    if (rpp != NULL) {
	    	        *rpp = (rs >= 0) ? ep->s : NULL ;
		    }
		    break ;
		}
	    } /* end while */
	    fifoelem_curend(fsp,&cur) ;
	} /* end if (cursor) */

	return ((rs >= 0) ? rl : rs) ;
}
/* end subroutine (fifoelem_finder) */


int fifoelem_curbegin(FIFOELEM *fsp,FIFOELEM_CURSOR *cp)
{

	if (fsp == NULL) return SR_FAULT ;

	if (fsp->magic != FIFOELEM_MAGIC) return SR_NOTOPEN ;

	cp->current = NULL ;
	return SR_OK ;
}
/* end subroutine (fifoelem_curbegin) */


int fifoelem_curend(FIFOELEM *fsp,FIFOELEM_CURSOR *cp)
{

	if (fsp == NULL) return SR_FAULT ;

	if (fsp->magic != FIFOELEM_MAGIC) return SR_NOTOPEN ;

	cp->current = NULL ;
	return SR_OK ;
}
/* end subroutine (fifoelem_curend) */


/* privatesubroutines */


static int entry_start(FIFOELEM_ENT *ep,void *sp,int sl)
{
	int		rs ;
	const char	*cp ;

	if (sl < 0) sl = strlen(sp) ;

	ep->previous = ep->next = NULL ;
	if ((rs = uc_mallocstrw(sp,sl,&cp)) >= 0) {
	    ep->slen = sl ;
	    ep->s = sp ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(FIFOELEM_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->s != NULL) {
	    rs1 = uc_free(ep->s) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->s = NULL ;
	}

	ep->slen = 0 ;
	return rs ;
}
/* end subroutine (entry_finish) */


static int defaultcmp(const void *a1p,const void *a2p)
{
	const void	**e1pp = (const void **) a1p ;
	const void	**e2pp = (const void **) a2p ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
		    cchar	*i1p = (cchar *) *e1pp ;
		    cchar	*i2p = (cchar *) *e2pp ;
	            rc = strcmp(i1p,i2p) ;
	        } else
		    rc = -1 ;
	    } else
		rc = 1 ;
	}
	return rc ;
}
/* end subroutine (defaultcmp) */


