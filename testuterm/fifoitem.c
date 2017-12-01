/* fifoitem */

/* FIFO string operations */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	- 2005-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object manages a FIFO of fixed sized entries and of fixed sized
        depth.


*******************************************************************************/


#define	FIFOITEM_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"fifoitem.h"


/* local defines */


/* type defs */

typedef int	(*cmpfun_t)(const void *,const void *) ;


/* external subroutines */


/* forward referecens */

int		fifoitem_del(FIFOITEM *,FIFOITEM_CUR *) ;
int		fifoitem_curbegin(FIFOITEM *,FIFOITEM_CUR *) ;
int		fifoitem_curend(FIFOITEM *,FIFOITEM_CUR *) ;
int		fifoitem_fetch(FIFOITEM *,FIFOITEM_CUR *,FIFOITEM_ENT **) ;

static int	entry_start(FIFOITEM_ENT *,const void *,int) ;
static int	entry_finish(FIFOITEM_ENT *) ;
static int	defaultcmp(cchar **,cchar **) ;


/* local variables */


/* exported subroutines */


int fifoitem_start(FIFOITEM *fsp)
{

	fsp->head = fsp->tail = NULL ;
	fsp->n = 0 ;
	fsp->magic = FIFOITEM_MAGIC ;
	return SR_OK ;
}
/* end subroutine (fifoitem_start) */


/* free up the entire vector string data structure object */
int fifoitem_finish(FIFOITEM *fsp)
{
	int		rs = SR_OK ;

	if (fsp == NULL) return SR_FAULT ;
	if (fsp->magic != FIFOITEM_MAGIC) return SR_NOTOPEN ;

	while ((rs = fifoitem_del(fsp,NULL)) >= 0) ;
	if (rs == SR_NOTFOUND) rs = SR_OK ;

	fsp->magic = 0 ;
	return rs ;
}
/* end subroutine (fifoitem_finish) */


/* add an element (to the tail) */
int fifoitem_ins(FIFOITEM *fsp,const void *sp,int sl)
{
	const int	esize = sizeof(FIFOITEM_ENT) ;
	int		rs ;
	void		*np ;

#if	CF_DEBUGS
	debugprintf("fifoitem_add: ent\n") ;
#endif

	if (fsp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (fsp->magic != FIFOITEM_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("fifoitem: s=%s len=%d\n",s,slen) ;
#endif

	if ((rs = uc_malloc(esize,&np)) >= 0) {
	    FIFOITEM_ENT	*ep = (FIFOITEM_ENT *) np ;
	    if ((rs = entry_start(ep,sp,sl)) >= 0) {
	        if (fsp->head == NULL) {
	            fsp->head = ep ;
	            fsp->tail = ep ;
	            ep->prev = ep->next = NULL ;
	        } else {
	            ep->prev = fsp->tail ;
	            (fsp->tail)->next = ep ;
	            fsp->tail = ep ;
	        } /* end if */
	        fsp->n += 1 ;
	    } /* end if (entry_start) */
	    if (rs < 0)
	        uc_free(np) ;
	} /* end if (m-a) */

	return (rs >= 0) ? fsp->n : rs ;
}
/* end subroutine (fifoitem_ins) */


/* remove from the head */
int fifoitem_rem(FIFOITEM *fsp,void *vbuf,int vlen)
{
	FIFOITEM_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		dl = 0 ;

#if	CF_DEBUGS
	debugprintf("fifoitem_remove: ent\n") ;
#endif

	if (fsp == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;
	if (fsp->magic != FIFOITEM_MAGIC) return SR_NOTOPEN ;

/* can we give the call an entry? */

	if (fsp->head == NULL)
	    return SR_NOENT ;

	ep = fsp->head ;
	if ((vlen >= 0) && (ep->dl > vlen))
	    return SR_OVERFLOW ;

	dl = ep->dl ;
	memcpy(vbuf,ep->dp,ep->dl) ;

	fsp->head = ep->next ;
	if (fsp->head == NULL) {
	    fsp->tail = NULL ;
	} else {
	    (fsp->head)->prev = NULL ;
	}

	rs1 = entry_finish(ep) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = uc_free(ep) ;
	if (rs >= 0) rs = rs1 ;

	fsp->n -= 1 ;
	return (rs >= 0) ? dl : rs ;
}
/* end subroutine (fifoitem_rem) */


/* fetch the next entry from the one under the cursor (or at the head) */
int fifoitem_fetch(FIFOITEM *fsp,FIFOITEM_CUR *curp,FIFOITEM_ENT **epp)
{
	FIFOITEM_ENT	*ep ;
	int		rs ;

	if (fsp == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;
	if (fsp->magic != FIFOITEM_MAGIC) return SR_NOTOPEN ;

/* OK, do the deed */

	if ((curp == NULL) || (curp->current == NULL)) {
	    ep = fsp->head ;
	} else {
	    ep = (curp->current)->next ;
	}

	if (curp != NULL)
	    curp->current = ep ;

	if (epp != NULL)
	    *epp = ep ;

	rs = (ep != NULL) ? ep->dl : SR_NOTFOUND ;
	return rs ;
}
/* end subroutine (fifoitem_fetch) */


/* delete the element that is under the cursor (or at the head) */
int fifoitem_del(FIFOITEM *fsp,FIFOITEM_CUR *curp)
{
	FIFOITEM_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (fsp == NULL) return SR_FAULT ;
	if (fsp->magic != FIFOITEM_MAGIC) return SR_NOTOPEN ;

/* OK, do the deed */

	if ((curp == NULL) || (curp->current == NULL)) {
	    ep = fsp->head ;
	} else {
	    ep = curp->current ;
	}

	if (ep != NULL) {

	    if (curp != NULL) {

	        if (ep->prev == NULL) {
	            fsp->head = ep->next ;
	        } else {
	            (ep->prev)->next = ep->next ;
	        }

	        if (ep->next == NULL) {
	            fsp->tail = ep->prev ;
	        } else {
	            (ep->next)->prev = ep->prev ;
	        }

	        curp->current = ep->prev ;

	    } else {

	        fsp->head = ep->next ;
	        if (fsp->head == NULL) {
	            fsp->tail = NULL ;
	        } else {
	            (fsp->head)->prev = NULL ;
	        }

	    } /* end if */

	    rs1 = entry_finish(ep) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = uc_free(ep) ;
	    if (rs >= 0) rs = rs1 ;

	    fsp->n -= 1 ;

	} else {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? fsp->n : rs ;
}
/* end subroutine (fifoitem_del) */


/* return the count of the number of items */
int fifoitem_count(FIFOITEM *fsp)
{

	if (fsp == NULL) return SR_FAULT ;
	if (fsp->magic != FIFOITEM_MAGIC) return SR_NOTOPEN ;

	return fsp->n ;
}
/* end subroutine (fifoitem_count) */


/* search for an element */
int fifoitem_finder(FIFOITEM *fsp,cchar *s,cmpfun_t cmpfunc,cchar **rpp)
{
	fifoitem_cur	cur ;
	int		rs ;
	int		dl = 0 ;

	if (fsp == NULL) return SR_FAULT ;
	if (fsp->magic != FIFOITEM_MAGIC) return SR_NOTOPEN ;

	if (cmpfunc == NULL)
	    cmpfunc = (cmpfun_t) defaultcmp ;

	if ((rs = fifoitem_curbegin(fsp,&cur)) >= 0) {
	    FIFOITEM_ENT	*ep ;
	    while ((rs = fifoitem_fetch(fsp,&cur,&ep)) >= 0) {
	        if ((*cmpfunc)(s,ep->dp) == 0) {
	            dl = ep->dl ;
	            if (rpp != NULL) {
	                cchar	*rp = (cchar *) ep->dp ;
	                *rpp = (rs >= 0) ? rp : NULL ;
	            }
	            break ;
	        }
	    } /* end while */
	    fifoitem_curend(fsp,&cur) ;
	} /* end if (cursor) */

	return (rs >= 0) ? dl : rs ;
}
/* end subroutine (fifoitem_finder) */


int fifoitem_curbegin(FIFOITEM *fsp,FIFOITEM_CUR *curp)
{

	if (fsp == NULL) return SR_FAULT ;
	if (fsp->magic != FIFOITEM_MAGIC) return SR_NOTOPEN ;

	curp->current = NULL ;
	return SR_OK ;
}
/* end subroutine (fifoitem_curbegin) */


int fifoitem_curend(FIFOITEM *fsp,FIFOITEM_CUR *curp)
{

	if (fsp == NULL) return SR_FAULT ;
	if (fsp->magic != FIFOITEM_MAGIC) return SR_NOTOPEN ;

	curp->current = NULL ;
	return SR_OK ;
}
/* end subroutine (fifoitem_curend) */


/* private subroutines */


static int entry_start(FIFOITEM_ENT *ep,const void *vp,int sl)
{
	int		rs ;
	const char	*sp = (const char *) vp ;
	const void	*dp ;

	if (sl < 0) sl = strlen(sp) ;

	ep->next = NULL ;
	ep->prev = NULL ;
	if ((rs = uc_mallocbuf(sp,sl,&dp)) >= 0) {
	    ep->dp = (const void *) dp ;
	    ep->dl = sl ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(FIFOITEM_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->dp != NULL) {
	    rs1 = uc_free(ep->dp) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->dp = NULL ;
	}

	ep->prev = ep->next = NULL ;
	return rs ;
}
/* end subroutine (entry_finish) */


static int defaultcmp(cchar **e1pp,cchar **e2pp)
{
	int		rc = 0 ;

	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp == NULL) {
	        rc = 1 ;
	    } else if (*e2pp == NULL) {
	        rc = -1 ;
	    } else {
	        rc = strcmp(*e1pp,*e2pp) ;
	    }
	}

	return rc ;
}
/* end subroutine (defaultcmp) */


