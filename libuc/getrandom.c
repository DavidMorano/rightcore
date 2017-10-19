/* getrandom */

/* Get-Random-data UNIX® System interposer */


#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a version of |getrandom(2)| that is preloaded to over-ride the
        standard UNIX® system version.

	Q. Is this multi-thread safe?
	A. Since it is a knock-off of an existing UNIX® system LIBC (3c)
	   subroutine that is already multi-thread safe -- then of course
	   it is!

	Q. Is this much slower than the default system version?
	A. No, not really.

	Q. How are we smarter than the default system version?
	A. Let me count the ways:
		+ value is cached!

	Q. Why did you not also interpose something for |sysconf(3c)|?
	A. Because we did not want to.

	Q. Why are you so smart?
	A. I do not know.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/random.h>
#include	<unistd.h>
#include	<string.h>
#include	<errno.h>
#include	<vsystem.h>
#include	<sigblock.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<randomvar.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOISE	5		/* interval for adding noise */
#define	TO_CAP		5		/* 5 seconds */

#ifndef	RANDOMDEV
#define	RANDOMDEV	"/dev/urandom"
#endif

#define	RBUFLEN		64

#ifndef	GRAND_RANDOM
#define	GRAND_RANDOM	(1<<1)
#define	GRAND_NONBLOCK	(1<<2)
#endif /* GRAND_RANDOM */

#define	GETRANDOM	struct getrandom_head


/* external subroutines */

extern int	msleep(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */

struct getrandom_head {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	void		*rv ;
	time_t		ti_noise ;	/* last addition of noise */
	volatile int	waiters ;
	volatile int	f_init ; 	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	volatile int	f_capture ;	/* capture flag */
} ;


/* forward references */

int		getrandom_init() ;
void		getrandom_fini() ;

static void	getrandom_atforkbefore() ;
static void	getrandom_atforkafter() ;

static int	getrandom_get(GETRANDOM *,char *,int,uint) ;
static int	getrandom_randomvar(GETRANDOM *) ;
static int	getrandom_geter(GETRANDOM *,char *,int,uint) ;
static int	getrandom_addnoise(GETRANDOM *,uint) ;

static int	getrandom_begin(GETRANDOM *) ;
static int	getrandom_end(GETRANDOM *) ;
static int	getrandom_capbegin(GETRANDOM *,int) ;
static int	getrandom_capend(GETRANDOM *) ;


/* local variables */

static GETRANDOM	getrandom_data ; /* zero-initialized */

static const char	*devs[] = {
	"/dev/urandom",
	"/dev/random",
	NULL
} ;


/* exported subroutines */


int getrandom(void *arbuf,size_t arlen,uint fl)
{
	GETRANDOM	*uip = &getrandom_data ;
	const int	rlen = (int) arlen ;
	int		rs ;
	char		*rbuf = (char *) arbuf ;
	if ((rs = getrandom_get(uip,rbuf,rlen,fl)) < 0) {
	    errno = (-rs) ;
	    rs = -1 ;
	}
	return rs ;
}
/* end subroutine (getrandom) */


int getentropy(void *rbuf,size_t rlen)
{
	int		rc ;
	if (rlen < 256) {
	    if ((rc = getrandom(rbuf,rlen,0)) >= 0) {
	        rc = 0 ;
	    }
	} else {
	    errno = EIO ;
	    rc = -1 ;
	}
	return rc ;
}
/* end subroutine (getentropy) */


int getrandom_init()
{
	GETRANDOM	*uip = &getrandom_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	    	    void	(*b)() = getrandom_atforkbefore ;
	    	    void	(*a)() = getrandom_atforkafter ;
	            if ((rs = uc_atfork(b,a,a)) >= 0) {
	                if ((rs = uc_atexit(getrandom_fini)) >= 0) {
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
		if (rs == SR_INTR) break ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (getrandom_init) */


void getrandom_fini()
{
	GETRANDOM	*uip = &getrandom_data ;
	if (uip->f_initdone) {
	    {
	        getrandom_end(uip) ;
	    }
	    {
	        void	(*b)() = getrandom_atforkbefore ;
	        void	(*a)() = getrandom_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(GETRANDOM)) ;
	} /* end if (was initialized) */
}
/* end subroutine (getrandom_fini) */


/* local subroutines */


int getrandom_get(GETRANDOM *uip,char *rbuf,int rlen,uint fl)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = getrandom_init()) >= 0) {
	        const int	to = TO_CAP ;
	        if ((rs = getrandom_capbegin(uip,to)) >= 0) {
		    if ((rs = getrandom_randomvar(uip)) >= 0) {
	                rs = getrandom_geter(uip,rbuf,rlen,fl) ;
			len = rs ;
		    } /* end if (getrandom_randomvar) */
	            rs1 = getrandom_capend(uip) ;
	            if (rs >= 0) rs = rs1 ;
		} /* end if (getrandom_cap) */
	    } /* end if (getrandom_init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

#if	CF_DEBUGS
	debugprintf("getrandom_get: ret rs=%d c=>%t<\n",rs,rbuf,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getrandom_get) */


static int getrandom_randomvar(GETRANDOM *uip)
{
	int		rs = SR_OK ;
	if (uip->rv == NULL) {
	    rs = getrandom_begin(uip) ;
	}
	return rs ;
}
/* end subroutine (getrandom_randomvar) */


static int getrandom_geter(GETRANDOM *uip,char *rbuf,int rlen,uint fl)
{
	time_t		dt = time(NULL) ;
	int		rs = SR_OK ;
	int		rl = 0 ;
	if ((dt - uip->ti_noise) >= TO_NOISE) {
	    uip->ti_noise = dt ;
	    rs = getrandom_addnoise(uip,fl) ;
	} /* end if */
	if (rs >= 0) {
	    RANDOMVAR	*rvp = uip->rv ;
	    ULONG	uv ;
	    const int	usize = sizeof(ULONG) ;
	    int		i ;
	    while ((rs >= 0) && (rlen > 0)) {
		if ((rs = randomvar_getulong(rvp,&uv)) >= 0) {
		    for (i = 0 ; (rlen > 0) && (i < usize) ; i += 1) {
			rbuf[rl++] = (char) uv ;
			uv >>= 8 ;
			rlen -= 1 ;
		    } /* end for */
		} /* end if (randomvar_getulong) */
	    } /* end while */
	} /* end if */
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (getrandom_geter) */


static int getrandom_addnoise(GETRANDOM *uip,uint fl)
{
	const int	ri = MKBOOL(fl&GRAND_RANDOM) ;
	const int	of = O_RDONLY ;
	cchar		*dev ;
	int		rs ;
	dev = devs[ri] ;
	if ((rs = u_open(dev,of,0666)) >= 0) {
	    const int	rlen = RBUFLEN ;
	    const int	fd = rs ;
	    char	rbuf[RBUFLEN+1] ;
	    if ((rs = u_read(fd,rbuf,rlen)) >= 0) {
		RANDOMVAR	*rvp = uip->rv ;
		const int	len = rs ;
		rs = randomvar_addnoise(rvp,rbuf,len) ;
	    } /* end if (read) */
	    u_close(fd) ;
	} /* end if (file-random) */
	return rs ;
}
/* end subroutine (getrandom_addnoise) */


static int getrandom_begin(GETRANDOM *uip)
{
	int		rs = SR_OK ;

	if (uip->rv == NULL) {
	    const int	osize = sizeof(RANDOMVAR) ;
	    void	*p ;
	    if ((rs = uc_libmalloc(osize,&p)) >= 0) {
	        RANDOMVAR	*rvp = (RANDOMVAR *) p ;
	        if ((rs = randomvar_start(rvp,0,0)) >= 0) {
	            uip->rv = p ;
		}
	        if (rs < 0)
	            uc_libfree(p) ;
	    } /* end if (memory-allocation) */
	} /* end if (needed initialization) */

	return rs ;
}
/* end subroutine (getrandom_begin) */


static int getrandom_end(GETRANDOM *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->rv != NULL) {
	    RANDOMVAR	*rvp = uip->rv ;
	    rs1 = randomvar_finish(rvp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(uip->rv) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->rv = NULL ;
	}
	return rs ;
}
/* end subroutine (getrandom_end) */


static int getrandom_capbegin(GETRANDOM *uip,int to)
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
/* end subroutine (getrandom_capbegin) */


static int getrandom_capend(GETRANDOM *uip)
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
/* end subroutine (getrandom_capend) */


static void getrandom_atforkbefore()
{
	GETRANDOM	*uip = &getrandom_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (getrandom_atforkbefore) */


static void getrandom_atforkafter()
{
	GETRANDOM	*uip = &getrandom_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (getrandom_atforkafter) */


