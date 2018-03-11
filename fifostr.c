/* fifostr */

/* FIFO string operations */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		0		/* extra safety */


/* revision history:

	= 1999-12-09, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-09-12, David A­D­ Morano
	Small interface change to |fifostr_entread()|.

*/

/* Copyright © 1999,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages a FIFO of strings.


*******************************************************************************/


#define	FIFOSTR_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"fifostr.h"


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


/* forward references */

int		fifostr_curbegin(FIFOSTR *,FIFOSTR_CUR *) ;
int		fifostr_curend(FIFOSTR *,FIFOSTR_CUR *) ;
int		fifostr_del(FIFOSTR *,FIFOSTR_CUR *) ;

static int	fifostr_mat(FIFOSTR *,FIFOSTR_ENT *) ;

#ifdef	COMMENT
static int	cmpdefault() ;
#endif


/* exported subroutines */


int fifostr_start(FIFOSTR *op)
{

	if (op == NULL) return SR_FAULT ;

	op->head = op->tail = NULL ;
	op->ic = 0 ;
	op->cc = 0 ;
	op->magic = FIFOSTR_MAGIC ;
	return SR_OK ;
}
/* end subroutine (fifostr_start) */


/* free up the entire object */
int fifostr_finish(FIFOSTR *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;

	while ((rs = fifostr_del(op,NULL)) >= 0) ;
	if (rs == SR_NOTFOUND) rs = SR_OK ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (fifostr_finish) */


/* add a string (to the tail) */
int fifostr_add(FIFOSTR *op,cchar sp[],int sl)
{
	FIFOSTR_ENT	*ep ;
	int		rs ;
	int		size ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

	if (sp == NULL) return SR_FAULT ;

	if (sl < 0)
	    sl = strlen(sp) ;

/* allocate and fill-in the new entry */

	size = sizeof(FIFOSTR_ENT) + (sl + 1) ;
	if ((rs = uc_malloc(size,&ep)) >= 0) {

	    ep->slen = sl ;
	    {
	        char	*bp = (char *) ep ;
	        bp += sizeof(FIFOSTR_ENT) ;
	        strwcpy(bp,sp,sl) ;
	    }

/* link in the new entry */

	    ep->prev = ep->next = NULL ;
	    if (op->head == NULL) {
	        op->head = ep ;
	        op->tail = ep ;
	    } else {
	        ep->prev = op->tail ;
	        (op->tail)->next = ep ;
	        op->tail = ep ;
	    } /* end if */

	    op->ic += 1 ;
	    op->cc += sl ;

	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("fifostr_add: ret rs=%d n=%u\n",rs,op->ic) ;
#endif

	return (rs >= 0) ? op->ic : rs ;
}
/* end subroutine (fifostr_add) */


/* read the head entry of the FIFO */
int fifostr_headread(FIFOSTR *op,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		sl ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

/* are there any entries available? */

	if (op->head != NULL) {
	    FIFOSTR_ENT	*ep ;
	    ep = op->head ;
	    sl = ep->slen ;
	    if (rbuf != NULL) {
	        const char	*sp = (const char *) ep ;
	        sp += sizeof(FIFOSTR_ENT) ;
	        rs = snwcpy(rbuf,rlen,sp,sl) ;
	    }
	} else {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (fifostr_headread) */


/* get the length of the item on the head */
int fifostr_headlen(FIFOSTR *op)
{
	int		rs = SR_OK ;
	int		sl = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

/* are there any entries available? */

	if (op->head != NULL) {
	    FIFOSTR_ENT	*ep = op->head ;
	    sl = ep->slen ;
	} else {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (fifostr_headlen) */


/* read the specified entry */
int fifostr_entread(FIFOSTR *op,char *rbuf,int rlen,int n)
{
	FIFOSTR_ENT	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		sl = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

	if (n < 0)
	    return SR_INVALID ;

/* are there any entries available? */

#ifdef	OPTIONAL
	if (op->head == NULL)
	    return SR_NOTFOUND ;
#endif

	ep = op->head ;
	for (i = 0 ; (i < n) && (ep != NULL) ; i += 1) {
	    ep = ep->next ;
	}

	if (ep != NULL) {
	    sl = ep->slen ;
	    if (rbuf != NULL) {
	        const char	*sp = (const char *) ep ;
	        sp += sizeof(FIFOSTR_ENT) ;
	        rs = snwcpy(rbuf,rlen,sp,sl) ;
	    }
	} else {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (fifostr_entread) */


/* get the length of the specified entry */
int fifostr_entlen(FIFOSTR *op,int n)
{
	FIFOSTR_ENT	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		sl = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

	if (n < 0)
	    return SR_INVALID ;

/* are there any entries available? */

#ifdef	OPTIONAL
	if (op->head == NULL)
	    return SR_NOTFOUND ;
#endif

	ep = op->head ;
	for (i = 0 ; (i < n) && (ep != NULL) ; i += 1) {
	    ep = ep->next ;
	}

	if (ep != NULL) {
	    sl = ep->slen ;
	} else {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (fifostr_entlen) */


/* remove a string (from the head) */
int fifostr_remove(FIFOSTR *op,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		sl = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

/* do we have any entries? */

	if (op->head != NULL) {
	    FIFOSTR_ENT	*ep = op->head ;
	    sl = ep->slen ;
	    if (rbuf != NULL) {
	        const char	*sp = (const char *) ep ;
	        sp += sizeof(FIFOSTR_ENT) ;
	        rs = snwcpy(rbuf,rlen,sp,sl) ;
	    }
/* unlink and delete the entry */
	    if (rs >= 0) {
	        op->head = ep->next ;
	        if (op->head == NULL) {
	            op->tail = NULL ;
	        } else {
	            (op->head)->prev = NULL ;
	        }
	        rs = uc_free(ep) ;
	        op->ic -= 1 ;
	        op->cc -= sl ;
	    } /* end if (successful removal) */
	} else {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (fifostr_remove) */


int fifostr_curbegin(FIFOSTR *op,FIFOSTR_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL)
	    return SR_FAULT ;

	curp->current = NULL ;
	return SR_OK ;
}
/* end subroutine (fifostr_curbegin) */


int fifostr_curend(FIFOSTR *op,FIFOSTR_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	curp->current = NULL ;
	return SR_OK ;
}
/* end subroutine (fifostr_curend) */


/* fetch the next entry from the one under the cursor (or at the head) */
int fifostr_enum(FIFOSTR *op,FIFOSTR_CUR *curp,char *rbuf,int rlen)
{
	FIFOSTR_ENT	*ep ;
	int		rs = SR_OK ;
	int		sl = 0 ;
	const char	*sp ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

/* OK, do the deed */

	if ((curp == NULL) || (curp->current == NULL)) {
	    ep = op->head ;
	} else {
	    if ((rs = fifostr_mat(op,curp->current)) >= 0) {
	        ep = (curp->current)->next ;
	    }
	} /* end if */

	if (rs >= 0) {
	    if (curp != NULL)
	        curp->current = ep ;
	    if (ep != NULL) {
	        sl = ep->slen ;
	        if (rbuf != NULL) {
	            sp = (const char *) ep ;
	            sp += sizeof(FIFOSTR_ENT) ;
	            rs = snwcpy(rbuf,rlen,sp,sl) ;
	        }
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (fifostr_enum) */


/* delete a string that is under the cursor (or at the head) */
int fifostr_del(FIFOSTR *op,FIFOSTR_CUR *curp)
{
	FIFOSTR_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

/* OK, do the deed */

	if ((curp == NULL) || (curp->current == NULL)) {
	    ep = op->head ;
	} else {
	    ep = curp->current ;
	}

	if (ep != NULL) {

	    sl = ep->slen ;
	    if (curp != NULL) {

	        if (ep->prev == NULL) {
	            op->head = ep->next ;
	        } else {
	            (ep->prev)->next = ep->next ;
	        }

	        if (ep->next == NULL) {
	            op->tail = ep->prev ;
	        } else {
	            (ep->next)->prev = ep->prev ;
	        }

	        curp->current = ep->prev ;

	    } else {

	        op->head = ep->next ;
	        if (op->head == NULL) {
	            op->tail = NULL ;
	        } else {
	            (op->head)->prev = NULL ;
	        }

	    } /* end if */

	    rs1 = uc_free(ep) ;
	    if (rs >= 0) rs = rs1 ;

	    op->ic -= 1 ;
	    op->cc -= sl ;

	} else {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? op->ic : rs ;
}
/* end subroutine (fifostr_del) */


/* return the count of the number of items in this list */
int fifostr_count(FIFOSTR *op)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;
#endif

	return op->ic ;
}
/* end subroutine (fifostr_count) */


#ifdef	COMMENT

/* search for a string in the FIFO string object */
int fifostr_finder(FIFOSTR *op,char *s,int (*cmpfunc)(),char **rpp)
{
	fifostr_cur	cur ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != FIFOSTR_MAGIC) return SR_NOTOPEN ;

	if (cmpfunc == NULL) cmpfunc = cmpdefault ;

	if ((rs = fifostr_curbegin(op,&cur)) >= 0) {
	    const char	*rp ;
	    while ((rs = fifostr_enum(op,&cur,&rp)) >= 0) {
	        if ((*cmpfunc)(s,rp) == 0) break ;
	    } /* end while */
	    if (rpp != NULL) *rpp = (rs >= 0) ? rp : NULL ;
	    fifostr_curend(op,&cur) ;
	} /* end if (fifostr-cur) */

	return rs ;
}
/* end subroutine (fifostr_finder) */

#endif /* COMMENT */


/* private subroutines */


static int fifostr_mat(FIFOSTR *op,FIFOSTR_ENT *mep)
{
	FIFOSTR_ENT	*ep = op->head ;
	int		rs = SR_NOTFOUND ;

	while (ep != NULL) {
	    if (ep == mep) {
	        rs = SR_OK ;
	        break ;
	    }
	    ep = ep->next ;
	} /* end while */

	return rs ;
}
/* end subroutine (fifostr_mat) */


