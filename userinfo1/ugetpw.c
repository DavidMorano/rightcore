/* ugetpw */

/* get UNIX® password entries (w/ cache) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module serves as a per-process cache for UNIX® password entries.

	Since we are basically dealing with global data, we need to make the
	establishment of it multi-thread safe.  We also want fork safety.  Yes,
	we want everything, including cleanup on module unloading (since, yes,
	we could all be inside a loadable and unloadble module!).  For these
	purposes we employ the basic (and not so basic) means of accomplishing
	this.  See the code for our various machinations.

	¥ Note: The type 'int' is assumed to be atomic for multithreaded
	synchronization purposes.  The atomic type |sig_atomic_t| is (just) an
	'int', so we do not feel too guilty ourselves about using an 'int' as
	an interlock.

	Notes:

	Q. The old age question: do these (public) subroutines need to be
	multi-thread-safe?
	A. What do you think?

	Q. Why cannot we just use a POSIX® mutex-lock around the guts of
	the public subroutines?
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
#include	<pwd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<upwcache.h>
#include	<localmisc.h>

#include	"ugetpw.h"


/* local defines */

#define	UGETPW		struct ugetpw_head


/* external subroutines */

extern int	msleep(int) ;


/* local structures */

struct ugetpw_head {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	UPWCACHE	*pwc ;		/* PW cache (allocated) */
	int		max ;
	int		ttl ;
	volatile int	f_init ;	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	volatile int	f_capture ;	/* capture flag */
	volatile int	waiters ;
} ;


/* forward references */

int		ugetpw_init() ;
void		ugetpw_fini() ;

static int	ugetpw_begin(UGETPW *) ;
static int	ugetpw_end(UGETPW *) ;
static int	ugetpw_capbegin(UGETPW *,int) ;
static int	ugetpw_capend(UGETPW *) ;

static void	ugetpw_atforkbefore() ;
static void	ugetpw_atforkafter() ;


/* local variables */

static UGETPW		ugetpw_data ;	/* zero-initialized */


/* exported subroutines */


int ugetpw_init()
{
	UGETPW		*uip = &ugetpw_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	    	    void	(*b)() = ugetpw_atforkbefore ;
	    	    void	(*a)() = ugetpw_atforkafter ;
	            if ((rs = uc_atfork(b,a,a)) >= 0) {
	                if ((rs = uc_atexit(ugetpw_fini)) >= 0) {
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
/* end subroutine (ugetpw_init) */


void ugetpw_fini()
{
	UGETPW		*uip = &ugetpw_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        if (uip->pwc != NULL) {
	            ugetpw_end(uip) ;
	        }
	    }
	    {
	        void	(*b)() = ugetpw_atforkbefore ;
	        void	(*a)() = ugetpw_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(UGETPW)) ;
	} /* end if (was initialized) */
}
/* end subroutine (ugetpw_fini) */


int ugetpw_name(struct passwd *pwp,char *pwbuf,int pwlen,const char *un)
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (pwp == NULL) return SR_FAULT ;
	if (pwbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (un[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("ugetpw_name: u=%s\n",un) ;
	debugprintf("ugetpw_name: test passwd\n") ;
	memset(pwp,0,sizeof(struct passwd)) ;
	debugprintf("ugetpw_name: test pwbuf\n") ;
	memset(pwbuf,0,pwlen) ;
	debugprintf("ugetpw_name: cont\n") ;
#endif

	memset(pwp,0,sizeof(struct passwd)) ;

	if ((rs = ugetpw_init()) >= 0) {
	    UGETPW	*uip = &ugetpw_data ;
	    if ((rs = ugetpw_capbegin(uip,-1)) >= 0) {

	        if (uip->pwc == NULL) rs = ugetpw_begin(uip) ;

	        if (rs >= 0) {
	            UPWCACHE	*pwcp = (UPWCACHE *) uip->pwc ;

#if	CF_DEBUGS
		    debugprintf("ugetpw_name: upwcache_lookup()\n") ;
#endif

	            rs = upwcache_lookup(pwcp,pwp,pwbuf,pwlen,un) ;
	            len = rs ;
	        } /* end if (lookup) */

	        rs1 = ugetpw_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture-exclusion) */
	} /* end if (init) */

#if	CF_DEBUGS
	debugprintf("ugetpw_name: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (ugetpw_name) */


int ugetpw_uid(struct passwd *pwp,char *pwbuf,int pwlen,uid_t uid)
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (pwp == NULL) return SR_FAULT ;
	if (pwbuf == NULL) return SR_FAULT ;

	memset(pwp,0,sizeof(struct passwd)) ;

	if ((rs = ugetpw_init()) >= 0) {
	    UGETPW	*uip = &ugetpw_data ;
	    if ((rs = ugetpw_capbegin(uip,-1)) >= 0) {

	        if (uip->pwc == NULL) rs = ugetpw_begin(uip) ;

	        if (rs >= 0) {
	            UPWCACHE	*pwcp = (UPWCACHE *) uip->pwc ;
	            rs = upwcache_uid(pwcp,pwp,pwbuf,pwlen,uid) ;
	            len = rs ;
	        } /* end if (get by UID) */

	        rs1 = ugetpw_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture-exclusion) */
	} /* end if (init) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (ugetpw_uid) */


int ugetpw_stats(UGETPW_STATS *usp)
{
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if (usp == NULL) return SR_FAULT ;

	memset(usp,0,sizeof(UGETPW_STATS)) ;

	if ((rs = ugetpw_init()) >= 0) {
	    UGETPW	*uip = &ugetpw_data ;
	    if ((rs = ugetpw_capbegin(uip,-1)) >= 0) {

	        if (uip->pwc == NULL) rs = ugetpw_begin(uip) ;

	        if (rs >= 0) {
	            UPWCACHE		*pwcp = (UPWCACHE *) uip->pwc ;
	            UPWCACHE_STATS	s ;
	            if ((rs = upwcache_stats(pwcp,&s)) >= 0) {
	                usp->max = uip->max ;
	                usp->ttl = uip->ttl ;
	                usp->nent = s.nentries ;
	                usp->acc = s.total ;
	                usp->phit = s.phits ;
	                usp->nhit = s.nhits ;
	                usp->pmis = s.pmisses ;
	                usp->nmis = s.nmisses ;
	                n = s.nentries ;
	            } /* end if (upwcache-stats) */
	        } /* end if */

	        rs1 = ugetpw_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture-exclusion) */
	} /* end if (init) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ugetpw_stats) */


/* local subroutines (or "private"?) */


static int ugetpw_capbegin(UGETPW *uip,int to)
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
/* end subroutine (ugetpw_capbegin) */


static int ugetpw_capend(UGETPW *uip)
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
/* end subroutine (ugetpw_capend) */


static int ugetpw_begin(UGETPW *uip)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("ugetpw_begin: ent\n") ;
#endif

	if (uip->pwc == NULL) {
	    const int	size = sizeof(UPWCACHE) ;
	    void	*p ;
	    if ((rs = uc_libmalloc(size,&p)) >= 0) {
	        const int	max = UGETPW_MAX ;
	        const int	ttl = UGETPW_TTL ;
	        UPWCACHE	*pwcp = (UPWCACHE *) p ;
	        if ((rs = upwcache_start(pwcp,max,ttl)) >= 0) {
	            uip->pwc = pwcp ;
	            uip->max = max ;
	            uip->ttl = ttl ;
		}
	        if (rs < 0)
	            uc_libfree(p) ;
	    } /* end if (memory-allocation) */
	} /* end if (needed initialization) */

#if	CF_DEBUGS
	debugprintf("ugetpw_begin: max=%u\n",uip->max) ;
	debugprintf("ugetpw_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ugetpw_begin) */


static int ugetpw_end(UGETPW *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (uip->pwc != NULL) {
	    UPWCACHE	*pwcp = (UPWCACHE *) uip->pwc ;
	    rs1 = upwcache_finish(pwcp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_libfree(uip->pwc) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->pwc = NULL ;
	}

	return rs ;
}
/* end subroutine (ugetpw_end) */


static void ugetpw_atforkbefore()
{
	UGETPW		*uip = &ugetpw_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (ugetpw_atforkbefore) */


static void ugetpw_atforkafter()
{
	UGETPW		*uip = &ugetpw_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (ugetpw_atforkafter) */


