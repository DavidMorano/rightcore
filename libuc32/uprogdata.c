/* uprogdata */

/* set or get some program (process) data */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2004-11-22, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We get (or set) some program (process) data.

	Synopsis:

	int uprogdataget(spec,rbuf,rlen)
	char		rbuf[] ;
	int		rlen ;
	int		spec ;

	Arguments:

	rbuf		buffer to receive the requested cluster name
	rlen		length of supplied buffer
	spec		integer specifying what data to get

	Returns:

	>=0		string length of cluster name
	SR_OK		if OK
	SR_NOTFOUND	if could not get something needed for correct operation
	SR_ISDIR	database file was a directory (admin error)
	<0		some other error

	Design note:

	Q. Is this (mess) multi-thread safe?
	A. Duh!

	Q. Does this need to be so complicated?
	A. Yes.

	Q. Was the amount of complication warranted by the need?
	A. Looking at it now ... maybe not.

	Q. Were there any alternatives?
	A. Yes; the predicessor to this present implementation was an 
	   implementation that was quite simple, but it had a lot of static
	   memory storage.  It was the desire to eliminate the static memory
	   storage that led to this present implementation.

	Q. Are there ways to clean this up further?
	A. Probably, but it looks I have already done more to this simple
	   function than may have been ever warranted to begin with!

	Q. Did these subroutines have to be Async-Signal-Safe?
	A. Not really.

	Q. Then why did you do it?
	A. The system-call |uname(2)| is Async-Signal-Safe.  Since these
	   subroutines sort of look like |uname(2)| (of a sort), I thought 
	   it was a good idea.

	Q. Was it really a good idea?
	A. I guess not.


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

#define	UPROGDATA	struct uprogdata_head
#define	UPROGDATA_ENT	struct uprogdata_e

#define	TO_CAP		5		/* 5 seconds */
#define	TO_TTL		(2*3600)	/* two hours */


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

struct uprogdata_head {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	VARRAY		*ents ;
	volatile int	waiters ;
	volatile int	f_init ; 	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	volatile int	f_capture ;	/* capture flag */
} ;

struct uprogdata_e {
	time_t		et ;		/* entry-time (load-time) */
	const char	*vp ;
	int		vl ;
	int		ttl ;		/* time-to-live */
} ;


/* forward references */

int		uprogdata_init() ;
void		uprogdata_fini() ;

static void	uprogdata_atforkbefore() ;
static void	uprogdata_atforkafter() ;

static int	uprogdata_struct(UPROGDATA *) ;
static int	uprogdata_begin(UPROGDATA *) ;
static int	uprogdata_end(UPROGDATA *) ;
static int	uprogdata_entfins(UPROGDATA *) ;
static int	uprogdata_capbegin(UPROGDATA *,int) ;
static int	uprogdata_capend(UPROGDATA *) ;
static int	uprogdata_seter(UPROGDATA *,int,const char *,int,int) ;
static int	uprogdata_geter(UPROGDATA *,int,char *,int) ;

static int entry_start(UPROGDATA_ENT *,const char *,int,int) ;
static int entry_reload(UPROGDATA_ENT *,const char *,int,int) ;
static int entry_finish(UPROGDATA_ENT *) ;


/* local variables */

static UPROGDATA	uprogdata_data ; /* zero-initialized */


/* exported subroutines */


int uprogdata_init()
{
	UPROGDATA	*uip = &uprogdata_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	            void	(*b)() = uprogdata_atforkbefore ;
	            void	(*a)() = uprogdata_atforkafter ;
	            if ((rs = uc_atfork(b,a,a)) >= 0) {
	                if ((rs = uc_atexit(uprogdata_fini)) >= 0) {
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
/* end subroutine (uprogdata_init) */


void uprogdata_fini()
{
	UPROGDATA	*uip = &uprogdata_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        uprogdata_end(uip) ;
	    }
	    {
	        void	(*b)() = uprogdata_atforkbefore ;
	        void	(*a)() = uprogdata_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(UPROGDATA)) ;
	} /* end if (was initialized) */
}
/* end subroutine (uprogdata_fini) */


int uprogdata_set(int di,cchar cbuf[],int clen,int ttl)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;

	if (cbuf == NULL) return SR_FAULT ;

	if (di < 0) return SR_INVALID ;
	if (cbuf[0] == '\0') return SR_INVALID ;

	if (ttl < 0) ttl = TO_TTL ;

#if	CF_DEBUGS
	debugprintf("uprogdata_set: ent di=%u c=>%t<\n",di,cbuf,clen) ;
#endif

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = uprogdata_init()) >= 0) {
		UPROGDATA	*uip = &uprogdata_data ;
		if ((rs = uprogdata_capbegin(uip,-1)) >= 0) {
		    if ((rs = uprogdata_struct(uip)) >= 0) {
			rs = uprogdata_seter(uip,di,cbuf,clen,ttl) ;
		    } /* end if (uprogdata_struct) */
		    rs1 = uprogdata_capend(uip) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (uprogdata_cap) */
	    } /* end if (uprogdata_init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

#if	CF_DEBUGS
	debugprintf("uprogdata_set: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uprogdata_set) */


int uprogdata_get(int di,char rbuf[],int rlen)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;

	if (di < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uprogdata_get: ent di=%u\n",di) ;
#endif

	rbuf[0] = '\0' ;
	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = uprogdata_init()) >= 0) {
	        UPROGDATA	*uip = &uprogdata_data ;
	        if ((rs = uprogdata_capbegin(uip,-1)) >= 0) {
		    if ((rs = uprogdata_struct(uip)) >= 0) {
	                rs = uprogdata_geter(uip,di,rbuf,rlen) ;
			len = rs ;
		    } /* end if (uprogdata_struct) */
	            rs1 = uprogdata_capend(uip) ;
	            if (rs >= 0) rs = rs1 ;
		} /* end if (uprogdata_cap) */
	    } /* end if (uprogdata_init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

#if	CF_DEBUGS
	debugprintf("uprogdata_get: ret rs=%d c=>%t<\n",rs,rbuf,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uprogdata_get) */


/* local subroutines */


static int uprogdata_struct(UPROGDATA *uip)
{
	int		rs = SR_OK ;
	if (uip->ents == NULL) {
	    rs = uprogdata_begin(uip) ;
	}
	return rs ;
}
/* end subroutine (uprogdata_struct) */


static int uprogdata_begin(UPROGDATA *uip)
{
	int		rs = SR_OK ;

	if (uip->ents == NULL) {
	    const int	osize = sizeof(VARRAY) ;
	    void	*p ;
	    if ((rs = uc_libmalloc(osize,&p)) >= 0) {
	        const int	esize = sizeof(UPROGDATA_ENT) ;
	        const int	n = 4 ;
	        VARRAY		*ents = (VARRAY *) p ;
	        if ((rs = varray_start(ents,esize,n)) >= 0) {
	            uip->ents = ents ;
		}
	        if (rs < 0)
	            uc_libfree(p) ;
	    } /* end if (memory-allocation) */
	} /* end if (needed initialization) */

	return rs ;
}
/* end subroutine (uprogdata_begin) */


static int uprogdata_end(UPROGDATA *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->ents != NULL) {
	    VARRAY	*ents = (VARRAY *) uip->ents ;
	    rs1 = uprogdata_entfins(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = varray_finish(ents) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_libfree(uip->ents) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->ents = NULL ;
	}
	return rs ;
}
/* end subroutine (uprogdata_end) */


static int uprogdata_entfins(UPROGDATA *uip)
{
	VARRAY		*vap = (VARRAY *) uip->ents ;
	UPROGDATA_ENT	*ep ;
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
/* end subroutine (uprogdata_entfins) */


static int uprogdata_capbegin(UPROGDATA *uip,int to)
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
/* end subroutine (uprogdata_capbegin) */


static int uprogdata_capend(UPROGDATA *uip)
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
/* end subroutine (uprogdata_capend) */


static int uprogdata_seter(UPROGDATA *uip,int di,cchar cbuf[],int clen,int ttl)
{
	VARRAY		*vap = (VARRAY *) uip->ents ;
	UPROGDATA_ENT	*ep ;
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
/* end subroutine (uprogdata_seter) */


static int uprogdata_geter(UPROGDATA *uip,int di,char rbuf[],int rlen)
{
	VARRAY		*vap = (VARRAY *) uip->ents ;
	UPROGDATA_ENT	*ep ;
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
/* end subroutine (uprogdata_geter) */


static void uprogdata_atforkbefore()
{
	UPROGDATA	*uip = &uprogdata_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (uprogdata_atforkbefore) */


static void uprogdata_atforkafter()
{
	UPROGDATA	*uip = &uprogdata_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (uprogdata_atforkafter) */


static int entry_start(UPROGDATA_ENT *ep,const char *vp,int vl,int ttl)
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


static int entry_finish(UPROGDATA_ENT *ep)
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


static int entry_reload(UPROGDATA_ENT *ep,const char *vp,int vl,int ttl)
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


