/* utmpacc */

/* UNIX® UTMP access management */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module serves as a per-process cache for UNIX® UTMP information.

	Since we are basically dealing with global data, we need to make the
	establishment of it multi-thread safe.  We also want fork safety.  Yes,
	we want everything, including cleanup on module unloading (since, yes,
	we could all be inside a loadable and unloadble module!).  For these
	purposes we employ the basic (and not so basic) means of accomplishing
	this.  See the code for our various machinations.

	+ descriptions

	Name:

	utmpacc_users

	Synopsis:

	int utmpacc_users(int w)

	Arguments:

	w		which user logins to count:
				0	normal users only
				1	normal users + login gettys
				2	normal users, login gettys, init procs

	Returns:

	-		count of requested user types


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>		/* for |getsid(3c)| */
#include	<string.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<sigblock.h>
#include	<filemap.h>
#include	<localmisc.h>

#include	"utmpacc.h"


/* local defines */

#define	UTMPACC		struct utmpacc_head
#define	UTMPACC_ITEM	struct utmpacc_i
#define	UTMPACC_REC	struct utmpacc_r

/* intervals (seconds) */
#define	UTMPACC_INTBOOT		(5*3600)
#define	UTMPACC_INTRUNLEVEL	5
#define	UTMPACC_INTUSERS	10
#define	UTMPACC_INTENT		10
#define	UTMPACC_TOCAP		(5*60)


/* typedefs */

typedef struct futmpx	futmpx_t ;


/* external subroutines */

extern int	msleep(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct utmpacc_r {
	UTMPACC_ENT	e ;
	time_t		ti_create ;
	time_t		ti_access ;
	uint		wcount ;
} ;

struct utmpacc_i {
	uint		t ;		/* create-time */
	uint		v ;		/* value */
} ;

struct utmpacc_head {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	void		*cache ;	/* cache (allocated) */
	UTMPACC_ITEM	btime ;
	UTMPACC_ITEM	runlevel ;
	UTMPACC_ITEM	nusers ;
	UTMPACC_ITEM	ent ;
	int		max ;
	int		ttl ;
	volatile int	waiters ;
	volatile int	f_init ; 	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	volatile int	f_capture ;
} ;


/* forward references */

static int	utmpacc_capbegin(UTMPACC *,int) ;
static int	utmpacc_capend(UTMPACC *) ;
static int	utmpacc_begin(UTMPACC *) ;
static int	utmpacc_end(UTMPACC *) ;

static int	utmpacc_scan(UTMPACC *,time_t) ;
static int	utmpacc_getusers(UTMPACC *,time_t,int) ;
static int	utmpacc_getentsid(UTMPACC *,time_t,UTMPACC_ENT *,
			char *,int,pid_t) ;

static void	utmpacc_atforkbefore() ;
static void	utmpacc_atforkafter() ;


/* local variables */

static UTMPACC		utmpacc_data ; /* zero-initialized */


/* exported subroutines */


int utmpacc_init()
{
	UTMPACC		*uip = &utmpacc_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	            void	(*b)() = utmpacc_atforkbefore ;
	            void	(*a)() = utmpacc_atforkafter ;
	            if ((rs = uc_atfork(b,a,a)) >= 0) {
	                if ((rs = uc_atexit(utmpacc_fini)) >= 0) {
	                    f = TRUE ;
	                    uip->f_initdone = TRUE ;
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
/* end subroutine (utmpacc_init) */


void utmpacc_fini()
{
	UTMPACC		*uip = &utmpacc_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        if (uip->cache != NULL) {
	            utmpacc_end(uip) ;
	        }
	    }
	    {
	        void	(*b)() = utmpacc_atforkbefore ;
	        void	(*a)() = utmpacc_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(UTMPACC)) ;
	} /* end if (was initialized) */
}
/* end subroutine (utmpacc_fini) */


int utmpacc_boottime(time_t *tp)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;

	if (tp == NULL) return SR_FAULT ;

	*tp = 0 ;
	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = utmpacc_init()) >= 0) {
	        UTMPACC		*uip = &utmpacc_data ;
	        if ((rs = utmpacc_capbegin(uip,-1)) >= 0) {
	            time_t	dt = time(NULL) ;
	            const int	to = UTMPACC_INTBOOT ;

	            if ((dt - uip->btime.t) >= to) {
	                rs = utmpacc_scan(uip,dt) ;
	            } /* end if (timed-out) */

	            rs1 = utmpacc_capend(uip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (capture-exclusion) */
	        if (rs >= 0) *tp = uip->btime.v ;
	    } /* end if (init) */
	    rs1 = sigblock_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

#if	CF_DEBUGS
	debugprintf("utmpacc_boottime: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (utmpacc_boottime) */


int utmpacc_runlevel()
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = utmpacc_init()) >= 0) {
	        UTMPACC		*uip = &utmpacc_data ;
	        if ((rs = utmpacc_capbegin(uip,-1)) >= 0) {
	            time_t	dt = time(NULL) ;
	            const int	to = UTMPACC_INTRUNLEVEL ;

	            if ((dt - uip->runlevel.t) >= to) {
	                rs = utmpacc_scan(uip,dt) ;
	            } /* end if */
	            if (rs >= 0) n = uip->runlevel.v ;

	            rs1 = utmpacc_capend(uip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (capture-exclusion) */
	    } /* end if (init) */
	    rs1 = sigblock_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (utmpacc_runlevel) */


int utmpacc_users(int w)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if (w < 0) return SR_INVALID ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = utmpacc_init()) >= 0) {
	        UTMPACC		*uip = &utmpacc_data ;
	        if ((rs = utmpacc_capbegin(uip,-1)) >= 0) {
	            time_t	dt = time(NULL) ;
	            const int	to = UTMPACC_INTUSERS ;

	            if ((dt - uip->nusers.t) >= to) {
	                rs = utmpacc_getusers(uip,dt,w) ;
	            } /* end if */
	            if (rs >= 0) n = uip->nusers.v ;

	            rs1 = utmpacc_capend(uip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (capture-exclusion) */
	    } /* end if (init) */
	    rs1 = sigblock_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (utmpacc_users) */


int utmpacc_entsid(UTMPACC_ENT *uep,char *uebuf,int uelen,pid_t sid)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;

	if (uep == NULL) return SR_FAULT ;
	if (uebuf == NULL) return SR_FAULT ;

	memset(uep,0,sizeof(UTMPACC_ENT)) ;
	uebuf[0] = '\0' ;

	if (sid <= 0) sid = getsid(0) ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = utmpacc_init()) >= 0) {
	        UTMPACC		*uip = &utmpacc_data ;
	        if ((rs = utmpacc_capbegin(uip,-1)) >= 0) {
	            {
	                time_t	dt = time(NULL) ;
	                rs = utmpacc_getentsid(uip,dt,uep,uebuf,uelen,sid) ;
	            }
	            rs1 = utmpacc_capend(uip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (capture-exclusion) */
	    } /* end if (init) */
	    rs1 = sigblock_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	if ((rs >= 0) && (uep->line == NULL)) rs = SR_NOTFOUND ;

	return rs ;
}
/* end subroutine (utmpacc_entsid) */


int utmpacc_stats(UTMPACC_STATS *usp)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if (usp == NULL) return SR_FAULT ;

	memset(usp,0,sizeof(UTMPACC_STATS)) ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = utmpacc_init()) >= 0) {
	        UTMPACC		*uip = &utmpacc_data ;
	        if ((rs = utmpacc_capbegin(uip,-1)) >= 0) {

	            if (uip->cache == NULL) {
			rs = utmpacc_begin(uip) ;
		    }

	            if (rs >= 0) {
	                usp->max = uip->max ;
	                usp->ttl = uip->ttl ;
	                n = uip->max ;
	            } /* end if */

	            rs1 = utmpacc_capend(uip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (capture-exclusion) */
	    } /* end if (init) */
	    rs1 = sigblock_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (utmpacc_stats) */


/* local subroutines */


/* ARGSUSED */
static int utmpacc_begin(UTMPACC *uip)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("utmpacc_begin: ent\n") ;
#endif

	if (uip == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("utmpacc_begin: max=%u\n",uip->max) ;
	debugprintf("utmpacc_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (utmpacc_begin) */


static int utmpacc_end(UTMPACC *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (uip->cache != NULL) {
	    rs1 = uc_libfree(uip->cache) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->cache = NULL ;
	} /* end if (initialized) */

	return rs ;
}
/* end subroutine (utmpacc_end) */


static int utmpacc_scan(UTMPACC *uip,time_t dt)
{
	FILEMAP		f ;
	const size_t	max = INT_MAX ;
	const int	of = O_RDONLY ;
	int		rs ;
	int		rs1 ;
	const char	*fn = UTMPACC_DEFUTMP ;

#if	CF_DEBUGS
	debugprintf("utmpacc_scan: fn=%s\n",fn) ;
#endif

	if ((rs = filemap_open(&f,fn,of,max)) >= 0) {
	    const futmpx_t	*up ;
	    const int		usize = sizeof(struct futmpx) ;
	    int			c = 0 ;

#if	CF_DEBUGS
	    debugprintf("utmpacc_scan: reading\n") ;
#endif
	    while ((rs = filemap_read(&f,usize,&up)) == usize) {
	        if (up->ut_type == UTMPACC_TRUNLEVEL) {
	            const int	eterm = up->ut_exit.e_termination ;
	            uip->runlevel.v = MKCHAR(eterm) ;
	            uip->runlevel.t = dt ;
	            c += 1 ;
	        }
	        if (up->ut_type == UTMPACC_TBOOTTIME) {
	            uip->btime.v = up->ut_tv.tv_sec ;
	            uip->btime.t = dt ;
	            c += 1 ;
	        }
	        if (c >= 2) break ;
	    } /* end while */

	    rs1 = filemap_close(&f) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filemap) */

#if	CF_DEBUGS
	debugprintf("utmpacc_scan: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (utmpacc_scan) */


/* ARGSUSED */
static int utmpacc_getusers(UTMPACC *uip,time_t dt,int w)
{
	FILEMAP		f ;
	const size_t	max = INT_MAX ;
	const int	of = O_RDONLY ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	const char	*fn = UTMPACC_DEFUTMP ;

	if ((rs = filemap_open(&f,fn,of,max)) >= 0) {
	    const futmpx_t	*up ;
	    const int		usize = sizeof(struct futmpx) ;

	    while ((rs = filemap_read(&f,usize,&up)) == usize) {
	        if (up->ut_type == UTMPACC_TRUNLEVEL) {
	            const int	eterm = up->ut_exit.e_termination ;
	            uip->runlevel.v = MKCHAR(eterm) ;
	            uip->runlevel.t = dt ;
	        }
	        if (up->ut_type == UTMPACC_TBOOTTIME) {
	            uip->btime.v = up->ut_tv.tv_sec ;
	            uip->btime.t = dt ;
	        }
	        if (up->ut_type == UTMPACC_TUSERPROC) c += 1 ;
		if ((w == 1) && (up->ut_type == UTMPACC_TLOGINPROC)) c += 1 ;
		if ((w == 2) && (up->ut_type == UTMPACC_TINITPROC)) c += 1 ;
	    } /* end while */

	    rs1 = filemap_close(&f) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filemap) */

	if (rs >= 0) {
	    uip->nusers.v = c ;
	    uip->nusers.t = dt ;
	}

#if	CF_DEBUGS
	debugprintf("utmpacc_users: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (utmpacc_getusers) */


static int utmpacc_getentsid(uip,dt,uep,uebuf,uelen,sid)
UTMPACC		*uip ;
time_t		dt ;
UTMPACC_ENT	*uep ;
char		uebuf[] ;
int		uelen ;
pid_t		sid ;
{
	FILEMAP		f ;
	const size_t	max = INT_MAX ;
	const int	of = O_RDONLY ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	const char	*fn = UTMPACC_DEFUTMP ;

	if ((rs = filemap_open(&f,fn,of,max)) >= 0) {
	    const futmpx_t	*up ;
	    const int		usize = sizeof(struct futmpx) ;

	    while ((rs = filemap_read(&f,usize,&up)) == usize) {
#if	CF_DEBUGS
	        debugprintf("utmpacc_getentsid: filemap_read() rs=%d\n",rs) ;
#endif
	        if (up->ut_type == UTMPACC_TRUNLEVEL) {
	            const int	eterm = up->ut_exit.e_termination ;
	            uip->runlevel.v = MKCHAR(eterm) ;
	            uip->runlevel.t = dt ;
	        }
	        if (up->ut_type == UTMPACC_TBOOTTIME) {
	            uip->btime.v = up->ut_tv.tv_sec ;
	            uip->btime.t = dt ;
	        }
	        if (up->ut_type == UTMPACC_TUSERPROC) {
	            if (up->ut_user[0] != '\0') {
	                c += 1 ;
	                if (up->ut_pid == sid)  {
	                    uip->ent.t = dt ;
	                    rs = utmpaccent_load(uep,uebuf,uelen,up) ;
	                }
	            } /* end if (non-nul) */
	        } /* end if (user-process) */
	    } /* end while (reading UTMPX entries) */

	    {
	        uip->nusers.v = c ;
	        uip->nusers.t = dt ;
	    }

	    rs1 = filemap_close(&f) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filemap) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (utmpacc_getentsid) */


static int utmpacc_capbegin(UTMPACC *uip,int to)
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
/* end subroutine (utmpacc_capbegin) */


static int utmpacc_capend(UTMPACC *uip)
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
/* end subroutine (utmpacc_capend) */


static void utmpacc_atforkbefore()
{
	UTMPACC		*uip = &utmpacc_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (utmpacc_atforkbefore) */


static void utmpacc_atforkafter()
{
	UTMPACC		*uip = &utmpacc_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (utmpacc_atforkafter) */


