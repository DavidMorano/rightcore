/* uc_memalloc (3uc) */
/* lang=C89 */

/* interface component for UNIX® library-3c */
/* memory allocation facility */


#define	CF_DEBUGN	0		/* non-switchable debug print-outs */
#define	CF_LIBMALLOC	0		/* using 'libmalloc(3malloc)' */
#define	CF_NORESERVE	1		/* do not reserve mapped memory */
#define	CF_CLEARFREE	0		/* clear freed memory */


/* revision history:

	= 1998-03-85, David A­D­ Morano

	This subroutine was originally written.


	= 2001-02-01, David A­D­ Morano

	I expanded the storage table size somewhat for larger programs.  We are
	handling larger amounts of data now-a-days!


*/

/* Copyright © 1998,2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the friendly version of the standrd 'malloc(3c)' subroutine.
	These subroutines are not interpositioned on the standard version but
	rather have their own symbol names.  The main feature of these
	subroutines are to provide an optional tracking of the allocated memory
	usage.

	API:

	uc_mallset()	turns on memory tracking

	uc_malloc()	analogous to the standard 'malloc(3c)'

	uc_calloc()	analogous to the standard 'calloc(3c)'

	uc_free()	analogous to the standard 'free(3c)'

	uc_mallout()	returns how much memory has been allocated ("out")

	Notes: Because these subroutes are used *everywhere* they really have
	to be MT-safe.  They are not by themselves Async-Signal-Safe, but you
	can do that yourself if you want to.  Oh, they are Fork-Safe also.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#if	CF_LIBMALLOC
#include	<malloc.h>
#endif

#include	<sys/param.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"ucmallreg.h"


/* local defines */

#define	UCMEMALLOC	struct ucmemalloc
#define	UCMEMALLOC_ENT	struct ucmemalloc_ent

#define	NDF		"ucmemalloc.deb"

#define	LIBUC_ENTLEN	(20 * 1024 * 1024)

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	TO_AGAIN	(5 * 60)


/* external subroutines */

extern uint	nextpowtwo(int) ;
extern uint	uceil(uint,int) ;

extern int	msleep(int) ;
extern int	iceil(int,int) ;

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif


/* local structures */

struct ucmemalloc_ent {
	caddr_t		a ;
	int		size ;
	int		next ;
} ;

struct ucmemalloc {
	PTM		m ;		/* data mutex */
	UCMEMALLOC_ENT	*regs ;
	size_t		regs_size ;
	int		regs_total ;
	int		regs_used ;
	int		regs_usedmax ;
	int		err_underflow ;
	int		err_overflow ;
	int		err_noalloc ;
	int		err_nofree ;
	int		err_rs ;
	int		out_num ;
	int		out_size ;
	int		out_nummax ;
	int		out_sizemax ;
	int		pagesize ;
	volatile int	f_init ;
	volatile int	f_initdone ;
	volatile int	f_track ;
} ;


/* forward references */

static uint	mkhash(const void *) ;

int		ucmemalloc_init() ;

static int	ucmemalloc_memreg(const void *,int) ;
static int	ucmemalloc_memrel(const void *) ;
static int	ucmemalloc_mempresent(const void *) ;

static int	ucmemalloc_trackstart(UCMEMALLOC *,int) ;
static int	ucmemalloc_trackstarter(UCMEMALLOC *,int) ;
static int	ucmemalloc_trackfinish(UCMEMALLOC *) ;

static int	ucmemalloc_trackmalloc(UCMEMALLOC *,int,void *) ;
static int	ucmemalloc_trackrealloc(UCMEMALLOC *,const void *,int,void *) ;
static int	ucmemalloc_trackfree(UCMEMALLOC *,const void *) ;
static int	ucmemalloc_trackpresent(UCMEMALLOC *,const void *) ;
static int	ucmemalloc_trackout(UCMEMALLOC *,uint *) ;

static int	hashindex(uint,int) ;

static void	ucmemalloc_atforkbefore() ;
static void	ucmemalloc_atforkafter() ;

void		ucmemalloc_fini() ;


/* local vaiables */

static struct ucmemalloc ucmemalloc_data ; /* zero-initialized */


/* exported subroutines */


int ucmemalloc_init()
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        void	(*b)() = ucmemalloc_atforkbefore ;
	        void	(*a)() = ucmemalloc_atforkafter ;
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(ucmemalloc_fini)) >= 0) {
	                uip->f_initdone = TRUE ;
			f = TRUE ;
	            }
	            if (rs < 0)
	                uc_atforkrelease(b,a,a) ;
	        } /* end if (uc_atfork) */
	        if (rs < 0)
	            ptm_destroy(&uip->m) ;
	    } /* end if (ptm_create) */
	    if (rs < 0)
	        uip->f_init = FALSE ;
	} else {
	    while ((rs >= 0) && uip->f_init && (! uip->f_initdone)) {
	        rs = msleep(1) ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (ucmemalloc_init) */


void ucmemalloc_fini()
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    if (uip->f_track) {
	        ucmemalloc_trackfinish(uip) ;
	    }
	    {
	        void	(*b)() = ucmemalloc_atforkbefore ;
	        void	(*a)() = ucmemalloc_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(ucmemalloc_data)) ;
	} /* end if (atexit registered) */
}
/* end subroutine (ucmemalloc_fini) */


int uc_malloc(int size,void *vp)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	int		rs ;

	if (uip->f_track) {
	    rs = ucmemalloc_trackmalloc(uip,size,vp) ;
	} else {
	    rs = uc_libmalloc(size,vp) ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"uc_malloc: ret s=%u rs=%d\n",size,rs) ;
	if (rs >= 0)
	    nprintf(NDF,"uc_malloc: ret a=%p\n",(*((caddr_t *)vp))) ;
#endif

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_malloc) */


int uc_calloc(int nelem,int esize,void *vp)
{
	const int	size = (nelem*esize) ;
	return uc_malloc(size,vp) ;
}
/* end subroutine (uc_calloc) */


/* 'libmalloc(3malloc)' is brain-dead, doesn't have a 'valloc(3malloc)' */

#if	CF_LIBMALLOC 

int uc_valloc(int size,void *vp)
{
	return uc_malloc(size,vp) ;
}
/* end subroutine (uc_valloc) */

#else /* CF_LIBMALLOC */

int uc_valloc(int size,void *vp)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	int		rs ;

	if (uip->f_track) {
	    rs = ucmemalloc_trackmalloc(uip,size,vp) ;
	} else {
	    rs = uc_libmalloc(size,vp) ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"uc_valloc: ret s=%u rs=%d\n",size,rs) ;
	if (rs >= 0)
	    nprintf(NDF,"uc_valloc: ret a=%p\n",(*((caddr_t *)vp))) ;
#endif

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_valloc) */

#endif /* CF_LIBMALLOC */


int uc_realloc(const void *cp,int size,void *vp)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	int		rs ;

	if (uip->f_track) {
	    rs = ucmemalloc_trackrealloc(uip,cp,size,vp) ;
	} else {
	    rs = uc_librealloc(cp,size,vp) ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"uc_realloc: ret s=%u oa=%p rs=%d\n",size,cp,rs) ;
	if (rs >= 0)
	    nprintf(NDF,"uc_realloc: ret a=%p\n",(*((caddr_t *)vp))) ;
#endif /* CF_DEBUGN */

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_realloc) */


int uc_free(const void *vp)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	int		rs ;

	if (uip->f_track) {
	    rs = ucmemalloc_trackfree(uip,vp) ;
	} else {
	    rs = uc_libfree(vp) ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"uc_free: a=%p rs=%d\n",vp,rs) ;
#endif /* CF_DEBUGN */

	return rs ;
}
/* end subroutine (uc_free) */


int uc_mallset(int cmd)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	int		rs ;

	if ((rs = ucmemalloc_init()) >= 0) {
	    switch (cmd) {
	    case 0:
	        rs = ucmemalloc_trackfinish(uip) ;
	        break ;
	    case 1:
	        rs = ucmemalloc_trackstart(uip,cmd) ;
	        break ;
	    default:
	        rs = SR_NOTSUP ;
	        break ;
	    } /* end switch */
	} /* end if (init) */

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_mallset: ret rs=%d f_track=%u\n",
	    rs,uip->f_track) ;
#endif

	return rs ;
}
/* end subroutine (uc_mallset) */


int uc_mallout(uint *rp)
{
	int		rs ;

	if ((rs = ucmemalloc_init()) >= 0) {
	    UCMEMALLOC	*uip = &ucmemalloc_data ;
	    rs = ucmemalloc_trackout(uip,rp) ;
	} /* end if (init) */

	return rs ;
}
/* end subroutine (uc_mallout) */


int uc_mallinfo(uint *rp,int size)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	int		rs ;

	if (rp == NULL) return SR_FAULT ;

	if (size < (ucmallreg_overlast * sizeof(uint)))
	    return SR_OVERFLOW ;

	if ((rs = ucmemalloc_init()) >= 0) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) {
	        if ((rs = ptm_lock(&uip->m)) >= 0) {

	            rp[ucmallreg_outnum] = uip->out_num ;
	            rp[ucmallreg_outnummax] = uip->out_nummax ;
	            rp[ucmallreg_outsize] = uip->out_size ;
	            rp[ucmallreg_outsizemax] = uip->out_sizemax ;
	            rp[ucmallreg_used] = uip->regs_used ;
	            rp[ucmallreg_usedmax] = uip->regs_usedmax ;
	            rp[ucmallreg_under] = uip->err_underflow ;
	            rp[ucmallreg_over] = uip->err_overflow ;
	            rp[ucmallreg_notalloc] = uip->err_noalloc ;
	            rp[ucmallreg_notfree] = uip->err_nofree ;

	            ptm_unlock(&uip->m) ;
	        } /* end if (mutex) */
	        uc_forklockend() ;
	    } /* end if (forklock) */
	} /* end if (init) */

	return rs ;
}
/* end subroutine (uc_mallinfo) */


int uc_mallpresent(const void *a)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	int		rs = SR_OK ;

	if (uip->f_track) {
	    rs = ucmemalloc_trackpresent(uip,a) ;
	} /* end if (tracking) */

	return rs ;
}
/* end subroutine (uc_mallpresent) */


int ucmallreg_curbegin(UCMALLREG_CUR *curp)
{
	if (curp == NULL) return SR_FAULT ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (uc_ucmallreg_curbegin) */


int ucmallreg_curend(UCMALLREG_CUR *curp)
{
	if (curp == NULL) return SR_FAULT ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (uc_ucmallreg_curend) */


int ucmallreg_enum(UCMALLREG_CUR *curp,UCMALLREG_REG *rp)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	int		rs ;
	int		rsize = 0 ;

	if (curp == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if ((rs = ucmemalloc_init()) >= 0) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) {
	        if ((rs = ptm_lock(&uip->m)) >= 0) {
	            int	ci = (curp->i < 0) ? 0 : (curp->i + 1) ;

	            if (ci < uip->regs_total) {
	                while (ci < uip->regs_total) {
	                    if (uip->regs[ci].a != NULL) {
	                        rp->addr = uip->regs[ci].a ;
	                        rp->size = uip->regs[ci].size ;
	                        rsize = uip->regs[ci].size ;
	                        curp->i = ci ;
	                        break ;
	                    }
	                    ci += 1 ;
	                } /* end while */
	                if (ci >= uip->regs_total) rs = SR_NOTFOUND ;
	            } else
	                rs = SR_NOTFOUND ;

	            ptm_unlock(&uip->m) ;
	        } /* end if (mutex) */
	        uc_forklockend() ;
	    } /* end if (forklock) */
	} /* end if (init) */

	return (rs >= 0) ? rsize : rs ;
}
/* end subroutine (uc_mallreg) */


/* local subroutines */


static int ucmemalloc_trackstart(UCMEMALLOC *uip,int opts)
{
	int		rs = SR_OK ;

	if (! uip->f_track) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) {
	        if ((rs = ptm_lock(&uip->m)) >= 0) {

	            rs = ucmemalloc_trackstarter(uip,opts) ;

	            ptm_unlock(&uip->m) ;
	        } /* end if (mutex) */
	        uc_forklockend() ;
	    } /* end if (forklock) */
	} /* end if (was not previously tracking) */

	return rs ;
}
/* end subroutine (ucmemalloc_trackstart) */


/* ARGSUSED */
static int ucmemalloc_trackstarter(UCMEMALLOC *uip,int opts)
{
	const int	rtl = nextpowtwo(LIBUC_ENTLEN) ;
	int		rs ;
	int		size ;

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_trackstarter: ent\n") ;
#endif

	rs = uip->err_rs ;
	if (uip->err_rs < 0) goto ret0 ;
	if (uip->f_track) goto ret0 ;

	uip->f_track = TRUE ;
	uip->pagesize = getpagesize() ;

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_trackstarter: rtl=%u pagesize=%d\n",
	    rtl,uip->pagesize) ;
#endif

/* map the registration (hash) table */

	size = rtl * sizeof(UCMEMALLOC_ENT) ;

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_trackstarter: size=%u\n",size) ;
#endif

	{
	    size_t	ms = uceil(size,uip->pagesize) ;
	    int		mp = (PROT_READ | PROT_WRITE) ;
	    int		mf = 0 ;
	    void	*md ;

	    mf |= MAP_PRIVATE ;
	    mf |= MAP_ANON ;
#if	CF_NORESERVE && defined(MAP_NORESERVE)
	    mf |= MAP_NORESERVE ;
#endif
	    if ((rs = u_mmap(NULL,ms,mp,mf,-1,0L,&md)) >= 0) {
	        uip->regs = (UCMEMALLOC_ENT *) md ;
	        uip->regs_size = ms ;
	        uip->regs_total = rtl ;
	    } /* end if (mmap) */

	} /* end block */

	if (rs < 0)
	    uip->err_rs = rs ;

ret0:

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_trackstarter: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ucmemalloc_trackstarter) */


static int ucmemalloc_trackfinish(UCMEMALLOC *uip)
{
	int		rs = SR_OK ;

	if (uip->f_track) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) {
	        if ((rs = ptm_lock(&uip->m)) >= 0) {

	            uip->f_track = FALSE ;
	            if (uip->regs != NULL) {
	                rs = u_munmap(uip->regs,uip->regs_size) ;
	                uip->regs = NULL ;
	                uip->regs_size = 0 ;
	                uip->regs_total = 0 ;
	            }

	            ptm_unlock(&uip->m) ;
	        } /* end if (mutex) */
	        uc_forklockend() ;
	    } /* end if (forklock) */
	} /* end if (was tracking) */

	return rs ;
}
/* end subroutine (ucmemalloc_trackfinish) */


static int ucmemalloc_trackmalloc(UCMEMALLOC *uip,int size,void *vp)
{
	int		rs ;

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_trackmalloc: err_rs=%d\n",
	    uip->err_rs) ;
#endif

	if ((rs = ucmemalloc_init()) >= 0) {
	    ulong	ma = (ulong) vp ;
	    if (ma >= uip->pagesize) {
	        if ((rs = uc_forklockbegin(-1)) >= 0) {
	            if ((rs = ptm_lock(&uip->m)) >= 0) {

	                if ((rs = uc_libmalloc(size,vp)) >= 0) {
	                    const caddr_t	a = *((caddr_t *)vp) ;
	                    rs = ucmemalloc_memreg(a,size) ;
	                }

	                ptm_unlock(&uip->m) ;
	            } /* end if (mutex) */
	            uc_forklockend() ;
	        } /* end if (forklock) */
	    } else
	        rs = SR_FAULT ;
	} /* end if (init) */

	return rs ;
}
/* end subroutine (ucmemalloc_trackmalloc) */


static int ucmemalloc_trackrealloc(uip,cp,size,vp)
UCMEMALLOC	*uip ;
const void	*cp ;
int		size ;
void		*vp ;
{
	int		rs ;
	int		rs1 ;

	if ((rs = ucmemalloc_init()) >= 0) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) {
	        if ((rs = ptm_lock(&uip->m)) >= 0) {

	            if ((rs = uc_librealloc(cp,size,vp)) >= 0) {
	                caddr_t	a = *((caddr_t *)vp) ;

	                rs1 = ucmemalloc_memrel(cp) ;
	                if (rs >= 0) rs = rs1 ;

	                rs1 = ucmemalloc_memreg(a,size) ;
	                if (rs >= 0) rs = rs1 ;

	            } /* end if (librealloc) */

	            ptm_unlock(&uip->m) ;
	        } /* end if (mutex) */
	        uc_forklockend() ;
	    } /* end if (forklock) */
	} /* end if (init) */

	return rs ;
}
/* end subroutine (ucmemalloc_trackrealloc) */


static int ucmemalloc_trackfree(UCMEMALLOC *uip,const void *vp)
{
	int		rs ;
	int		rs1 ;

	if ((rs = ucmemalloc_init()) >= 0) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) {
	        if ((rs = ptm_lock(&uip->m)) >= 0) {
	            if ((rs = ucmemalloc_memrel(vp)) >= 0) {
	                rs = uc_libfree(vp) ;
#if	CF_DEBUGN
	                nprintf(NDF,"ucmemalloc_trackfree: "
				"uc_libfree() rs=%d\n",rs) ;
#endif /* CF_DEBUGN */
	            }
	            rs1 = ptm_unlock(&uip->m) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (mutex) */
	        rs1 = uc_forklockend() ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (forklock) */
	} /* end if (init) */

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_trackfree: ret rs=%d\n",rs) ;
#endif /* CF_DEBUGN */

	return rs ;
}
/* end subroutine (ucmemalloc_trackfree) */


static int ucmemalloc_trackpresent(UCMEMALLOC *uip,const void *vp)
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if ((rs = ucmemalloc_init()) >= 0) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) {
	        if ((rs = ptm_lock(&uip->m)) >= 0) {
		    {
	                rs = ucmemalloc_mempresent(vp) ;
			len = rs ;
		    }
	            rs1 = ptm_unlock(&uip->m) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (mutex) */
	        rs1 = uc_forklockend() ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (forklock) */
	} /* end if (init) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (ucmemalloc_trackpresent) */


static int ucmemalloc_trackout(UCMEMALLOC *uip,uint *rp)
{
	int		rs ;
	int		out_size ;

	rs = uip->err_rs ;
	out_size = uip->out_size ;		/* fairly atomic (!) */
	if (uip->err_rs >= 0) {
	    rs = (out_size & INT_MAX) ;
	}

	if (rp != NULL) *rp = (rs >= 0) ? out_size : 0 ;

	return rs ;
}
/* end subroutine (ucmemalloc_trackout) */


/* register an allocation */
static int ucmemalloc_memreg(const void *a,int size)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	UCMEMALLOC_ENT	*tab ;
	uint		hv ;
	int		rs = SR_OK ;
	int		i ;
	int		phi, hi ;
	int		rtl ;

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_memreg: a=%p size=%u\n",a,size) ;
#endif /* CF_DEBUGN */

	uip->out_num += 1 ;
	if (uip->out_num > uip->out_nummax)
	    uip->out_nummax = uip->out_num ;

	if ((! uip->f_track) || (uip->regs == NULL)) goto ret0 ;
	rs = uip->err_rs ;
	if (uip->err_rs < 0) goto ret0 ;

/* search */

	tab = uip->regs ;
	rtl = uip->regs_total ;
	hv = mkhash(a) ;

	hi = hashindex(hv,rtl) ;

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_memreg: hi=%d\n",hi) ;
#endif /* CF_DEBUGN */

	phi = hi ;
	for (i = 0 ; hi && (a != tab[hi].a) && (i < rtl) ; i += 1) {
	    phi = hi ;
	    hi = tab[hi].next ;
	} /* end for */

/* take action on whether it was found or not */

	if (hi == 0) { /* nout found - insert (new) */

	    hi = phi ;
	    for (i = 0 ; (tab[hi].a != NULL) && (i < rtl) ; i += 1) {
	        hi = hashindex((hi + 1),rtl) ;
	    }

	    if (i < rtl) {

#if	CF_DEBUGN
	        nprintf(NDF,"ucmemalloc_memreg: inserting hi=%d\n",hi) ;
#endif /* CF_DEBUGN */

	        tab[hi].a = (caddr_t) a ;
	        tab[hi].size = size ;
	        tab[hi].next = 0 ;

	        uip->regs_used += 1 ;
	        if (uip->regs_used > uip->regs_usedmax)
	            uip->regs_usedmax = uip->regs_used ;

	    } else {

	        uip->err_overflow += 1 ;

	    } /* end if */

	} else { /* found - error (already there) */

#if	CF_DEBUGN
	    nprintf(NDF,"ucmemalloc_memreg: changing hi=%d\n",hi) ;
#endif /* CF_DEBUGN */

	    rs = SR_EXIST ;
	    uip->err_nofree += 1 ;
	    tab[hi].size = size ;
	    uip->err_rs = rs ;

	} /* end if */

	uip->out_size += size ;
	if (uip->out_size > uip->out_sizemax)
	    uip->out_sizemax = uip->out_size ;

ret0:

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_memreg: ret rs=%d\n",rs) ;
#endif /* CF_DEBUGN */

	return rs ;
}
/* end subroutine (ucmemalloc_memreg) */


/* release (or unregister) an allocation */
static int ucmemalloc_memrel(const void *a)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	UCMEMALLOC_ENT	*tab ;
	uint		hv ;
	int		rs = SR_OK ;
	int		i ;
	int		phi, hi ;
	int		rtl ;

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_memrel: a=%p\n",a) ;
#endif /* CF_DEBUGN */

	if ((! uip->f_track) || (uip->regs == NULL)) goto ret0 ;
	rs = uip->err_rs ;
	if (uip->err_rs < 0) goto ret0 ;

/* search */

	tab = uip->regs ;
	rtl = uip->regs_total ;
	hv = mkhash(a) ;

	hi = hashindex(hv,rtl) ;

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_memrel: hi=%u\n",hi) ;
#endif /* CF_DEBUGN */

	phi = hi ;
	for (i = 0 ; hi && (a != tab[hi].a) && (i < rtl) ; i += 1) {
	    phi = hi ;
	    hi = tab[hi].next ;
	} /* end for */

	if (i >= rtl) goto ret0 ; /* not-found due to no-space */

	if (hi > 0) { /* found */

#if	CF_DEBUGN
	    nprintf(NDF,"ucmemalloc_memrel: found size=%u\n",tab[hi].size) ;
#endif /* CF_DEBUGN */

	    uip->out_num -= 1 ;

/* remove */

	    tab[phi].next = tab[hi].next ;
	    uip->out_size -= tab[hi].size ;

#if	CF_CLEARFREE
	    {
	        caddr_t	a = (caddr_t) tab[hi].a ;
	        int	s = tab[hi].size ;
	        memset(a,0,s) ;
	    }
#endif /* CF_CLEARFREE */

	    if (uip->regs_used > 0) {
	        uip->regs_used -= 1 ;
	    } else {
	        uip->err_underflow += 1 ;
	    }

	    tab[hi].a = NULL ;
	    tab[hi].size = 0 ;
	    tab[hi].next = 0 ;

	} else { /* not-found */

	    rs = SR_BUGCHECK ;
	    uip->err_noalloc += 1 ;
	    uip->err_rs = rs ;

	} /* end if */

ret0:

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_memrel: ret rs=%d\n",rs) ;
#endif /* CF_DEBUGN */

	return rs ;
}
/* end subroutine (ucmemalloc_memrel) */


/* is an allocation present? */
static int ucmemalloc_mempresent(const void *a)
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
	int		rs = SR_OK ;

#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_mempresent: a=%p\n",a) ;
#endif /* CF_DEBUGN */

	if (uip->f_track && (uip->regs != NULL)) {
	    rs = uip->err_rs ;
	    if (uip->err_rs >= 0) {
	        UCMEMALLOC_ENT	*tab = uip->regs ;
	        uint		hv = mkhash(a) ;
	        const int	rtl = uip->regs_total ;
	        int		i ;
	        int		hi ;

	        hi = hashindex(hv,rtl) ;

	        for (i = 0 ; hi && (a != tab[hi].a) && (i < rtl) ; i += 1) {
	            hi = tab[hi].next ;
	        } /* end for */

	        rs = (i >= rtl) ? 0 : tab[hi].size ;

	    } /* end if (ok) */
	} /* end if (tracking enabled) */

	return rs ;
}
/* end subroutine (ucmemalloc_mempresent) */


static void ucmemalloc_atforkbefore()
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_atforkbefore: pid=%d\n",ucgetpid()) ;
#endif
	ptm_lock(&uip->m) ;
#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_atforkbefore: ret pid=%d\n",ucgetpid()) ;
#endif
}
/* end subroutine (ucmemalloc_atforkbefore) */


static void ucmemalloc_atforkafter()
{
	UCMEMALLOC	*uip = &ucmemalloc_data ;
#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_atforkafter: pid=%d\n",ucgetpid()) ;
#endif
	ptm_unlock(&uip->m) ;
#if	CF_DEBUGN
	nprintf(NDF,"ucmemalloc_atforkafter: ret pid=%d\n",ucgetpid()) ;
#endif
}
/* end subroutine (ucmemalloc_atforkafter) */


static uint mkhash(const void *a)
{
	ulong	v = (ulong) a ;
	return (uint) (v >> 3) ;
}
/* end subroutine (mkhash) */


static int hashindex(uint v,int n)
{
	int	hi = MODP2(v,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end subroutine (hashindex) */


