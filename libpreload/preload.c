/* preload */

/* set or get some program (process) data */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2004-11-22, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We serve out some (meant to be) preloaded subroutines.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<varray.h>
#include	<localmisc.h>


/* local defines */

#define	PRELOAD		struct preload_head
#define	PRELOAD_ENT	struct preload_e

#define	TO_CAP		5		/* 5 seconds */
#define	TO_TTL		(2*3600)	/* two hours */


/* pragmas */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	msleep(int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* local structures */

struct preload_head {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	VARRAY		*ents ;
	volatile int	waiters ;
	volatile int	f_init ; 	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	volatile int	f_capture ;	/* capture flag */
} ;

struct preload_e {
	time_t		et ;		/* entry-time (load-time) */
	const char	*vp ;
	int		vl ;
	int		ttl ;		/* time-to-live */
} ;


/* forward references */

int		preload_init() ;
void		preload_fini() ;

static void	preload_atforkbefore() ;
static void	preload_atforkafter() ;

int		preload_set(int,const char *,int,int) ;
int		preload_get(int,char *,int) ;

static int	preload_struct(PRELOAD *) ;
static int	preload_begin(PRELOAD *) ;
static int	preload_end(PRELOAD *) ;
static int	preload_entfins(PRELOAD *) ;
static int	preload_capbegin(PRELOAD *,int) ;
static int	preload_capend(PRELOAD *) ;
static int	preload_seter(PRELOAD *,int,const char *,int,int) ;
static int	preload_geter(PRELOAD *,int,char *,int) ;

static int	entry_start(PRELOAD_ENT *,cchar *,int,int) ;
static int	entry_reload(PRELOAD_ENT *,cchar *,int,int) ;
static int	entry_finish(PRELOAD_ENT *) ;


/* local variables */

static PRELOAD	preload_data ; /* zero-initialized */


/* exported subroutines */


int preload_init()
{
	PRELOAD		*uip = &preload_data ;
	int		rs = SR_OK ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	    	    void	(*b)() = preload_atforkbefore ;
	    	    void	(*a)() = preload_atforkafter ;
	            if ((rs = uc_atfork(b,a,a)) >= 0) {
	                if ((rs = uc_atexit(preload_fini)) >= 0) {
	                    rs = 1 ;
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
	return rs ;
}
/* end subroutine (preload_init) */


void preload_fini()
{
	PRELOAD	*uip = &preload_data ;
	if (uip->f_initdone) {
	    {
	        preload_end(uip) ;
	    }
	    {
	        void	(*b)() = preload_atforkbefore ;
	        void	(*a)() = preload_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(PRELOAD)) ;
	} /* end if (was initialized) */
}
/* end subroutine (preload_fini) */


int preload_set(int di,cchar *cbuf,int clen,int ttl)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;

	if (cbuf == NULL) return SR_FAULT ;

	if (di < 0) return SR_INVALID ;
	if (cbuf[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("preload_set: ent di=%u c=>%t<\n",di,cbuf,clen) ;
#endif

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = preload_init()) >= 0) {
		PRELOAD		*uip = &preload_data ;
		const int	to = TO_CAP ;
		if ((rs = preload_capbegin(uip,to)) >= 0) {
		    if ((rs = preload_struct(uip)) >= 0) {
			rs = preload_seter(uip,di,cbuf,clen,ttl) ;
		    } /* end if (preload_struct) */
		    rs1 = preload_capend(uip) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (preload_cap) */
	    } /* end if (preload_init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

#if	CF_DEBUGS
	debugprintf("preload_set: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (preload_set) */


int preload_get(int di,char *rbuf,int rlen)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;

	if (di < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("preload_get: ent di=%u\n",di) ;
#endif

	rbuf[0] = '\0' ;
	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = preload_init()) >= 0) {
	        PRELOAD		*uip = &preload_data ;
	        const int	to = TO_CAP ;
	        if ((rs = preload_capbegin(uip,to)) >= 0) {
		    if (uip->ents != NULL) {
	                rs = preload_geter(uip,di,rbuf,rlen) ;
			len = rs ;
		    } /* end if (entries-present) */
	            rs1 = preload_capend(uip) ;
	            if (rs >= 0) rs = rs1 ;
		} /* end if (preload_cap) */
	    } /* end if (preload_init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

#if	CF_DEBUGS
	debugprintf("preload_get: ret rs=%d c=>%t<\n",rs,rbuf,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (preload_get) */


/* local subroutines */


static int preload_struct(PRELOAD *uip)
{
	int		rs = SR_OK ;
	if (uip->ents == NULL) {
	    rs = preload_begin(uip) ;
	}
	return rs ;
}
/* end subroutine (preload_struct) */


static int preload_begin(PRELOAD *uip)
{
	int		rs = SR_OK ;

	if (uip->ents == NULL) {
	    const int	osize = sizeof(VARRAY) ;
	    void	*p ;
	    if ((rs = uc_libmalloc(osize,&p)) >= 0) {
	        VARRAY		*ents = (VARRAY *) p ;
	        const int	esize = sizeof(PRELOAD_ENT) ;
	        const int	n = 4 ;
	        if ((rs = varray_start(ents,esize,n)) >= 0) {
	            uip->ents = ents ;
		}
	        if (rs < 0)
	            uc_libfree(p) ;
	    } /* end if (memory-allocation) */
	} /* end if (needed initialization) */

	return rs ;
}
/* end subroutine (preload_begin) */


static int preload_end(PRELOAD *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->ents != NULL) {
	    VARRAY	*ents = (VARRAY *) uip->ents ;
	    rs1 = preload_entfins(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = varray_finish(ents) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_libfree(uip->ents) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->ents = NULL ;
	}
	return rs ;
}
/* end subroutine (preload_end) */


static int preload_entfins(PRELOAD *uip)
{
	VARRAY		*vap = (VARRAY *) uip->ents ;
	PRELOAD_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	for (i = 0 ; varray_enum(vap,i,&ep) >= 0 ; i += 1) { 
	    if (ep != NULL) {
	        rs1 = entry_finish(ep) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */
	return rs ;
}
/* end subroutine (preload_entfins) */


static int preload_capbegin(PRELOAD *uip,int to)
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
/* end subroutine (preload_capbegin) */


static int preload_capend(PRELOAD *uip)
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
/* end subroutine (preload_capend) */


static int preload_seter(PRELOAD *uip,int di,cchar *cbuf,int clen,int ttl)
{
	VARRAY		*vap = (VARRAY *) uip->ents ;
	PRELOAD_ENT	*ep ;
	int		rs ;

	if ((rs = varray_acc(vap,di,&ep)) > 0) {
	    rs = entry_reload(ep,cbuf,clen,ttl) ;
	} else if (rs == SR_OK) {
	    if ((rs = varray_mk(vap,di,&ep)) >= 0) {
		rs = entry_start(ep,cbuf,clen,ttl) ;
	    } /* end if (varray_mk) */
	} /* end if (array access) */

	return rs ;
}
/* end subroutine (preload_seter) */


static int preload_geter(PRELOAD *uip,int di,char *rbuf,int rlen)
{
	VARRAY		*vap = (VARRAY *) uip->ents ;
	PRELOAD_ENT	*ep ;
	int		rs ;
	int		len = 0 ;

	if ((rs = varray_acc(vap,di,&ep)) > 0) {
	    const time_t	dt = time(NULL) ;
	    if ((ep->et > 0) && ((dt-ep->et) < ep->ttl)) {
		rs = sncpy1w(rbuf,rlen,ep->vp,ep->vl) ;
		len = rs ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (preload_geter) */


static void preload_atforkbefore()
{
	PRELOAD		*uip = &preload_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (preload_atforkbefore) */


static void preload_atforkafter()
{
	PRELOAD		*uip = &preload_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (preload_atforkafter) */


static int entry_start(PRELOAD_ENT *ep,cchar *vp,int vl,int ttl)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	char		*bp ;
	if (vl < 0) vl = strlen(vp) ;
	if ((rs = uc_libmalloc((vl+1),&bp)) >= 0) {
	    ep->vp = bp ;
	    ep->vl = vl ;
	    strwcpy(bp,vp,vl) ;
	    ep->ttl = ttl ;
	    ep->et = dt ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(PRELOAD_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (ep->vp != NULL) {
	    rs1 = uc_libfree(ep->vp) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->vp = NULL ;
	}
	return rs ;
}
/* end subroutine (entry_finish) */


static int entry_reload(PRELOAD_ENT *ep,cchar *vp,int vl,int ttl)
{
	int		rs = SR_OK ;
	if (ep->vp != NULL) {
	    rs = uc_libfree(ep->vp) ;
	    ep->vp = NULL ;
	}
	if (rs >= 0) {
	    rs = entry_start(ep,vp,vl,ttl) ;
	}
	return rs ;
}
/* end subroutine (entry_reload) */


