/* poller */

/* poll manager */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2006-09-10, David A­D­ Morano
        I created this from hacking something that was similar that was
        originally created for a PCS program.

*/

/* Copyright © 2006 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages poll events.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<poll.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"poller.h"


/* local defines */

#define	POLLER_NDEF	10


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* local structures */


/* forward references */

int		poller_get(POLLER *,POLLER_SPEC *) ;

static int	poller_extend(POLLER *,int) ;
static int	poller_del(POLLER *,POLLER_SPEC *) ;

static int	ismatch(POLLER_SPEC *,POLLER_SPEC *) ;


/* local variables */


/* exported subroutines */


int poller_start(POLLER *op)
{
	const int	esize = sizeof(POLLER_SPEC) ;
	const int	n = POLLER_NDEF ;
	const int	vo = 0 ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(POLLER)) ;

	if ((rs = vecobj_start(&op->regs,esize,n,vo)) >= 0) {
	    op->magic = POLLER_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("poller_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (poller_start) */


int poller_finish(POLLER *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != POLLER_MAGIC) return SR_NOTOPEN ;

	rs1 = vecobj_finish(&op->regs) ;
	if (rs >= 0) rs = rs1 ;

	if (op->pa != NULL) {
	    rs1 = uc_free(op->pa) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pa = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (poller_finish) */


/* register an event */
int poller_reg(POLLER *op,POLLER_SPEC *rep)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (rep == NULL) return SR_FAULT ;

	if (op->magic != POLLER_MAGIC) return SR_NOTOPEN ;

	rs = vecobj_add(&op->regs,rep) ;

	return rs ;
}
/* end subroutine (poller_reg) */


/* cancel a previously registered event */
int poller_cancel(POLLER *op,POLLER_SPEC *cep)
{
	POLLER_SPEC	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (cep == NULL) return SR_FAULT ;

	if (op->magic != POLLER_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; vecobj_get(&op->regs,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        if (ismatch(ep,cep)) {
	            c += 1 ;
	            rs = vecobj_del(&op->regs,i--) ;
	        }
	    }
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (poller_cancel) */


/* cancel all previously registered events on an FD */
int poller_cancelfd(POLLER *op,int fd)
{
	POLLER_SPEC	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != POLLER_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; vecobj_get(&op->regs,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        if (ep->fd == fd) {
	            c += 1 ;
	            rs = vecobj_del(&op->regs,i--) ;
		}
	    }
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (poller_cancelfd) */


/* wait for an event to become ready */
int poller_wait(POLLER *op,POLLER_SPEC *rp,int mto)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->magic != POLLER_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("poller_wait: ent ready=%d\n",op->nready) ;
#endif

	if (op->nready > 0) {
	    rs = poller_get(op,rp) ;
	    c = rs ;
	} else {
	    VECOBJ	*rlp = &op->regs ;
	    int		n ;
	    n = vecobj_count(rlp) ;
	    if ((n > op->e) || (op->pa == NULL)) {
	        rs = poller_extend(op,n) ;
	    }
	    if (rs >= 0) {
		POLLER_SPEC	*ep = NULL ;
	        int		j = 0 ;
		int		i ;
	        op->n = n ;
	        for (i = 0 ; vecobj_get(rlp,i,&ep) >= 0 ; i += 1) {
	            if (ep != NULL) {
	                op->pa[j].fd = ep->fd ;
	                op->pa[j].events = ep->events ;
	                op->pa[j].revents = 0 ;
	                j += 1 ;
	            }
	        } /* end for */
	        if ((rs = u_poll(op->pa,n,mto)) > 0) {
	            op->nready = rs ;
	    	    rs = poller_get(op,rp) ;
		    c = rs ;
		} else if (rs == 0) {
	            op->nready = 0 ;
		} else if (rs == SR_INTR) {
	            op->nready = 0 ;
		    rs = SR_OK ;
		}
	    } /* end if (ok) */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("poller_wait: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (poller_wait) */


int poller_get(POLLER *op,POLLER_SPEC *rp)
{
	int		i ;
	int		c = 0 ;
	for (i = 0 ; i < op->n ; i += 1) {
	    if (op->pa[i].revents != 0) break ;
	} /* end for */
	if (i < op->n) {
	    *rp = op->pa[i] ;
	    poller_del(op,(op->pa + i)) ;
	    op->pa[i].fd = -1 ;
	    op->pa[i].events = 0 ;
	    op->pa[i].revents = 0 ;
	    c = op->nready ;
	    op->nready -= 1 ;
	} /* end if (result) */
	return c ;
}
/* end subroutine (poller_get) */


int poller_count(POLLER *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != POLLER_MAGIC) return SR_NOTOPEN ;

	rs = vecobj_count(&op->regs) ;

	return rs ;
}
/* end subroutine (poller_count) */


int poller_curbegin(POLLER *op,POLLER_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != POLLER_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (poller_curbegin) */


int poller_curend(POLLER *op,POLLER_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != POLLER_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (poller_curend) */


int poller_enum(POLLER *op,POLLER_CUR *curp,POLLER_SPEC *rp)
{
	POLLER_SPEC	*ep ;
	int		rs ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->magic != POLLER_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

	while ((rs = vecobj_get(&op->regs,i,&ep)) >= 0) {
	    if (ep != NULL) break ;
	    i += 1 ;
	} /* end while */

	if ((rs >= 0) && (ep != NULL)) {
	    *rp = *ep ;
	    curp->i = i ;
	}

	return rs ;
}
/* end subroutine (poller_enum) */


/* private subroutines */


static int poller_extend(POLLER *op,int n)
{
	const int	ne = MAX(n,1) ;
	int		rs = SR_OK ;

	if ((ne > op->e) || (op->pa == NULL)) {
	    int		size = ne * sizeof(POLLER_SPEC) ;
	    void	*p ;
	    if (op->pa != NULL) {
	        uc_free(op->pa) ;
	        op->pa = NULL ;
	    }
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        op->pa = p ;
	        op->e = ne ;
	    } else {
	        op->pa = NULL ;
	    }
	} /* end if (needed) */

	return (rs >= 0) ? ne : rs ;
}
/* end subroutine (poller_extend) */


/* delete a registration */
static int poller_del(POLLER *op,POLLER_SPEC *cep)
{
	POLLER_SPEC	*ep ;
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; vecobj_get(&op->regs,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        if (ismatch(ep,cep)) {
	            rs = vecobj_del(&op->regs,i) ;
	            break ;
		}
	    }
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (poller_del) */


static int ismatch(POLLER_SPEC *e1p,POLLER_SPEC *e2p)
{
	int		f = TRUE ;
	f = f && (e1p->fd == e2p->fd) ;
	f = f && (e1p->events == e2p->events) ;
	return f ;
}
/* end subroutine (ismatch) */


