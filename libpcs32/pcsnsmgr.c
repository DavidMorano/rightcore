/* pcsnsmgr */

/* PCS Name-Server Manager */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module manages the in-memory cache for the PCS Name-Server
	shared-object module.

	Notes:

	Q. The old age question: do these (public) subroutines need to be
	multi-thread-safe?
	A. What do you think?

	Q. Why cannot we just use a POSIX® mutex-lock around the guts of the
	public subroutines?
	A. Because those "guts" might do some complex operating-system-like
	things that would lead to a deadlock (because perhaps there is a
	|uc_fork(3uc)| in there, for example)!  It is *much* better to be safe
	than sorry.

	Q. Your (our) little "capture" mutex-lock implementation: why did you
	not just use a |lockrw(3dam)| object lock in exclusive (write-lock)
	mode -- because that is pretty much exactly what we have there?
	A. Oh, I do not know. I guess that originally I needed a more
	complicated lock of some sort (although that did not turn out to be the
	case), so going custom was what was needed.  As it is, a |lockrw(3dam)|
	would have been perfectly adequate.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<localmisc.h>

#include	"pcsnsmgr.h"
#include	"pcsnsrecs.h"


/* local defines */

#define	PCSNSMGR	struct pcsnsmgr_head


/* external subroutines */

extern int	msleep(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */

struct pcsnsmgr_head {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	PCSNSRECS	*recs ;		/* records (allocated) */
	int		max ;
	int		ttl ;
	volatile int	waiters ;
	volatile int	f_capture ;	/* capture flag */
	volatile int	f_init ;	/* race-condition, blah, blah */
	volatile int	f_initdone ;
} ;


/* forward references */

int		pcsnsmgr_init() ;
void		pcsnsmgr_fini() ;

static int	pcsnsmgr_begin(PCSNSMGR *) ;
static int	pcsnsmgr_end(PCSNSMGR *) ;
static int	pcsnsmgr_capbegin(PCSNSMGR *,int) ;
static int	pcsnsmgr_capend(PCSNSMGR *) ;

static void	pcsnsmgr_atforkbefore() ;
static void	pcsnsmgr_atforkafter() ;


/* local variables */

static PCSNSMGR		pcsnsmgr_data ;	/* zero-initialized */


/* exported subroutines */


int pcsnsmgr_init()
{
	PCSNSMGR	*uip = &pcsnsmgr_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	    	    void	(*b)() = pcsnsmgr_atforkbefore ;
	    	    void	(*a)() = pcsnsmgr_atforkafter ;
	            if ((rs = uc_atfork(b,a,a)) >= 0) {
	                if ((rs = uc_atexit(pcsnsmgr_fini)) >= 0) {
	                    uip->f_initdone = TRUE ;
			    f = TRUE ;
	                }
	                if (rs < 0)
	                    uc_atforkrelease(b,a,a) ;
	            } /* end if (uc_atfork) */
	            if (rs < 0)
	                ptc_destroy(&uip->c) ;
	        } /* end if (ptc_create) */
	        if (rs < 0)
	            ptm_destroy(&uip->m) ;
	    } /* end if (ptm_create) */
	    if (rs < 0)
	        uip->f_init = FALSE ;
	} else {
	    while ((rs >= 0) && uip->f_init && (! uip->f_initdone)) {
		rs = msleep(1) ;
		if (rs == SR_INTR) rs = SR_OK ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pcsnsmgr_init) */


void pcsnsmgr_fini()
{
	PCSNSMGR		*uip = &pcsnsmgr_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        if (uip->recs != NULL) {
	            pcsnsmgr_end(uip) ;
	        }
	    }
	    {
	        void	(*b)() = pcsnsmgr_atforkbefore ;
	        void	(*a)() = pcsnsmgr_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(PCSNSMGR)) ;
	} /* end if (was initialized) */
}
/* end subroutine (pcsnsmgr_fini) */


int pcsnsmgr_set(cchar *vbuf,int vlen,cchar *un,int w,int ttl)
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (vbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (un[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pcsnsmgr_set: ent u=%s w=%w\n",un,w) ;
#endif

	if ((rs = pcsnsmgr_init()) >= 0) {
	    PCSNSMGR	*uip = &pcsnsmgr_data ;
	    if ((rs = pcsnsmgr_capbegin(uip,-1)) >= 0) {

	        if (uip->recs == NULL) rs = pcsnsmgr_begin(uip) ;

	        if (rs >= 0) {
	            PCSNSRECS	*recsp = (PCSNSRECS *) uip->recs ;
	            rs = pcsnsrecs_store(recsp,vbuf,vlen,un,w,ttl) ;
		    len = rs ;
	        } /* end if (lookup) */

	        rs1 = pcsnsmgr_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture-exclusion) */
	} /* end if (init) */

#if	CF_DEBUGS
	debugprintf("pcsnsmgr_set: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pcsnsmgr_set) */


int pcsnsmgr_get(char *rbuf,int rlen,cchar *un,int w)
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("pcsnsmgr_get: ent u=%s w=%u\n",un,w) ;
#endif

	rbuf[0] = '\0' ;
	if ((rs = pcsnsmgr_init()) >= 0) {
	    PCSNSMGR	*uip = &pcsnsmgr_data ;
	    if ((rs = pcsnsmgr_capbegin(uip,-1)) >= 0) {

	        if (uip->recs != NULL) {
	            PCSNSRECS	*recsp = (PCSNSRECS *) uip->recs ;
	            if ((rs = pcsnsrecs_lookup(recsp,rbuf,rlen,un,w)) >= 0) {
	                len = rs ;
		    } else if (isNotPresent(rs)) {
			rs = SR_OK ;
		    }
	        } /* end if (active) */

	        rs1 = pcsnsmgr_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture-exclusion) */
	} /* end if (init) */

#if	CF_DEBUGS
	debugprintf("pcsnsmgr_get: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pcsnsmgr_get) */


int pcsnsmgr_invalidate(cchar *un,int w)
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (un == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("pcsnsmgr_invalidate: ent u=%s w=%u\n",un,w) ;
#endif

	if ((rs = pcsnsmgr_init()) >= 0) {
	    PCSNSMGR	*uip = &pcsnsmgr_data ;
	    if ((rs = pcsnsmgr_capbegin(uip,-1)) >= 0) {

	        if (uip->recs != NULL) {
	            PCSNSRECS	*recsp = (PCSNSRECS *) uip->recs ;
	            if ((rs = pcsnsrecs_invalidate(recsp,un,w)) >= 0) {
	                len = rs ;
		    } else if (isNotPresent(rs)) {
			rs = SR_OK ;
		    }
	        } /* end if (active) */

	        rs1 = pcsnsmgr_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture-exclusion) */
	} /* end if (init) */

#if	CF_DEBUGS
	debugprintf("pcsnsmgr_invalidate: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pcsnsmgr_invalidate) */


int pcsnsmgr_stats(PCSNSMGR_STATS *usp)
{
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if (usp == NULL) return SR_FAULT ;

	memset(usp,0,sizeof(PCSNSMGR_STATS)) ;

	if ((rs = pcsnsmgr_init()) >= 0) {
	    PCSNSMGR	*uip = &pcsnsmgr_data ;
	    if ((rs = pcsnsmgr_capbegin(uip,-1)) >= 0) {

	        if (uip->recs == NULL) rs = pcsnsmgr_begin(uip) ;

	        if (rs >= 0) {
	            PCSNSRECS		*recsp = (PCSNSRECS *) uip->recs ;
	            PCSNSRECS_ST	s ;
	            if ((rs = pcsnsrecs_stats(recsp,&s)) >= 0) {
	                usp->max = uip->max ;
	                usp->ttl = uip->ttl ;
	                usp->nent = s.nentries ;
	                usp->acc = s.total ;
	                usp->phit = s.phits ;
	                usp->nhit = s.nhits ;
	                usp->pmis = s.pmisses ;
	                usp->nmis = s.nmisses ;
	                n = s.nentries ;
	            } /* end if (pcsnsrecs-stats) */
	        } /* end if */

	        rs1 = pcsnsmgr_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture-exclusion) */
	} /* end if (init) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (pcsnsmgr_stats) */


/* local subroutines (or "private"?) */


static int pcsnsmgr_capbegin(PCSNSMGR *uip,int to)
{
	int		rs ;
	int		rs1 ;

	if ((rs = ptm_lockto(&uip->m,to)) >= 0) {
	    uip->waiters += 1 ;

	    while ((rs >= 0) && uip->f_capture) { /* busy */
	        rs = ptc_waiter(&uip->c,&uip->m,to) ;
	    } /* end while */

	    if (rs >= 0) {
	        uip->f_capture = TRUE ;
	    }

	    uip->waiters -= 1 ;
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (pcsnsmgr_capbegin) */


static int pcsnsmgr_capend(PCSNSMGR *uip)
{
	int		rs ;
	int		rs1 ;

	if ((rs = ptm_lock(&uip->m)) >= 0) {

	    uip->f_capture = FALSE ;
	    if (uip->waiters > 0) {
	        rs = ptc_signal(&uip->c) ;
	    }

	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (pcsnsmgr_capend) */


static void pcsnsmgr_atforkbefore()
{
	PCSNSMGR		*uip = &pcsnsmgr_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (pcsnsmgr_atforkbefore) */


static void pcsnsmgr_atforkafter()
{
	PCSNSMGR		*uip = &pcsnsmgr_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (pcsnsmgr_atforkafter) */


static int pcsnsmgr_begin(PCSNSMGR *uip)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("pcsnsmgr_begin: ent\n") ;
#endif

	if (uip->recs == NULL) {
	    const int	size = sizeof(PCSNSRECS) ;
	    void	*p ;
	    if ((rs = uc_libmalloc(size,&p)) >= 0) {
	        const int	max = PCSNSMGR_MAX ;
	        const int	ttl = PCSNSMGR_TTL ;
	        PCSNSRECS	*recsp = (PCSNSRECS *) p ;
	        if ((rs = pcsnsrecs_start(recsp,max,ttl)) >= 0) {
	            uip->recs = recsp ;
	            uip->max = max ;
	            uip->ttl = ttl ;
		}
	        if (rs < 0)
	            uc_libfree(p) ;
	    } /* end if (memory-allocation) */
	} /* end if (needed initialization) */

#if	CF_DEBUGS
	debugprintf("pcsnsmgr_begin: max=%u\n",uip->max) ;
	debugprintf("pcsnsmgr_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsnsmgr_begin) */


static int pcsnsmgr_end(PCSNSMGR *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("pcsnsmgr_end: ent\n") ;
#endif

	if (uip->recs != NULL) {
	    PCSNSRECS	*recsp = (PCSNSRECS *) uip->recs ;
	    rs1 = pcsnsrecs_finish(recsp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_libfree(uip->recs) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->recs = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("pcsnsmgr_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsnsmgr_end) */


