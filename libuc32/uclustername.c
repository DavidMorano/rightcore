/* uclustername */

/* set or get a cluster name given a nodename */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2004-11-22, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get a cluster name given a nodename.

	Synopsis:

	int uclusternameget(rbuf,rlen,nodename)
	char		rbuf[] ;
	int		rlen ;
	const char	nodename[] ;

	Arguments:

	rbuf		buffer to receive the requested cluster name
	rlen		length of supplied buffer
	nodename	nodename used to find associated cluster

	Returns:

	==0		could not get a cluster name
	>0		string length of cluster name
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
#include	<localmisc.h>


/* local defines */

#define	UCLUSTERNAME	struct clustername
#define	UCLUSTERNAME_A	struct clustername_a

#define	SUBINFO		struct subinfo

#define	TO_TTL		(2*3600) /* two hours */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	msleep(int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* local structures */

struct clustername {
	PTM		m ;		/* data mutex */
	time_t		et ;
	const char	*nn ;		/* node-name */
	const char	*cn ;		/* cluster-name */
	char		*a ;		/* memory-allocation */
	int		ttl ;		/* time-to-live */
	volatile int	f_init ; 	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	volatile int	f_allocget ;
} ;

struct clustername_a {
	time_t		et ;
	const char	*nn ;
	const char	*cn ;
	char		*a ;		/* memory-allocation */
	int		ttl ;		/* time-to-live */
} ;

struct subinfo {
	const char	*nn ;
	char		*rbuf ;
	time_t		dt ;
	int		rlen ;
	int		to ;
} ;


/* forward references */

int		uclustername_init() ;
void		uclustername_fini() ;

static void	uclustername_atforkbefore() ;
static void	uclustername_atforkafter() ;

static int	uclustername_end(UCLUSTERNAME *) ;
static int	uclustername_allocbegin(UCLUSTERNAME *,time_t,int) ;
static int	uclustername_allocend(UCLUSTERNAME *,UCLUSTERNAME_A *) ;

static int	subinfo_start(SUBINFO *,char *,int,const char *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_cacheget(SUBINFO *,UCLUSTERNAME *) ;
static int	subinfo_cacheset(SUBINFO *,UCLUSTERNAME *,int) ;


/* local variables */

static UCLUSTERNAME	uclustername_data ; /* zero-initialized */


/* exported subroutines */


int uclustername_init()
{
	UCLUSTERNAME	*uip = &uclustername_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        void	(*b)() = uclustername_atforkbefore ;
	        void	(*a)() = uclustername_atforkafter ;
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(uclustername_fini)) >= 0) {
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
		if (rs == SR_INTR) break ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uclustername_init) */


void uclustername_fini()
{
	UCLUSTERNAME	*uip = &uclustername_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        uclustername_end(uip) ;
	    }
	    {
	        void	(*b)() = uclustername_atforkbefore ;
	        void	(*a)() = uclustername_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(UCLUSTERNAME)) ;
	} /* end if (was initialized) */
}
/* end subroutine (uclustername_fini) */


int uclustername_get(char *rbuf,int rlen,cchar *nn)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;
	if (nn == NULL) return SR_FAULT ;

	if (nn[0] == '\0') return SR_INVALID ;

	rbuf[0] = '\0' ;
	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = uclustername_init()) >= 0) {
	        SUBINFO		si ;
	        if ((rs = subinfo_start(&si,rbuf,rlen,nn)) >= 0) {
	            UCLUSTERNAME	*uip = &uclustername_data ;
		    {
	                rs = subinfo_cacheget(&si,uip) ;
	                len = rs ;
		    }
	            rs1 = subinfo_finish(&si) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (subinfo) */
	    } /* end if (uclustername_init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uclustername_get) */


int uclustername_set(cchar *cbuf,int clen,cchar *nn,int to)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;

	if (cbuf == NULL) return SR_FAULT ;
	if (nn == NULL) return SR_FAULT ;

	if (cbuf[0] == '\0') return SR_INVALID ;
	if (nn[0] == '\0') return SR_INVALID ;

	if (clen < 0) clen = strlen(cbuf) ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = uclustername_init()) >= 0) {
	        SUBINFO		si ;
	        if ((rs = subinfo_start(&si,(char *) cbuf,clen,nn)) >= 0) {
	            UCLUSTERNAME	*uip = &uclustername_data ;
		    {
	                rs = subinfo_cacheset(&si,uip,to) ;
		    }
	            rs1 = subinfo_finish(&si) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (subinfo) */
	    } /* end if (uclustername_init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

	return rs ;
}
/* end subroutine (uclustername_set) */


/* local subroutines */


static int uclustername_end(UCLUSTERNAME *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->a != NULL) {
	    rs1 = uc_libfree(uip->a) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->a = NULL ;
	    uip->nn = NULL ;
	    uip->cn = NULL ;
	}
	return rs ;
}
/* end subroutine (uclustername_end) */


static void uclustername_atforkbefore()
{
	UCLUSTERNAME	*uip = &uclustername_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (uclustername_atforkbefore) */


static void uclustername_atforkafter()
{
	UCLUSTERNAME	*uip = &uclustername_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (uclustername_atforkafter) */


static int subinfo_start(SUBINFO *sip,char *rbuf,int rlen,cchar *nn)
{
	sip->to = TO_TTL ;
	sip->dt = time(NULL) ;
	sip->rbuf = rbuf ;
	sip->rlen = rlen ;
	sip->nn = nn ;
	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	if (sip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_cacheget(SUBINFO *sip,UCLUSTERNAME *uip)
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if ((rs = uc_forklockbegin(-1)) >= 0) {
	    if ((rs = ptm_lock(&uip->m)) >= 0) {
	        if (uip->a != NULL) {
	            if ((uip->et > 0) && ((sip->dt-uip->et) < uip->ttl)) {
	                if (strcmp(sip->nn,uip->nn) == 0) {
	                    rs = sncpy1(sip->rbuf,sip->rlen,uip->cn) ;
	                    len = rs ;
	                }
	            } /* end if (not timed-out) */
	        } /* end if (was loaded) */
	        rs1 = ptm_unlock(&uip->m) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = uc_forklockend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (forklock) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_cacheget) */


static int subinfo_cacheset(SUBINFO *sip,UCLUSTERNAME *uip,int ttl)
{
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	char		*aprev = NULL ;
#if	CF_DEBUGS
	debugprintf("uclustername/subinfo_cacheset: c=>%t<\n",
		sip->rbuf,sip->rlen) ;
#endif
	if (ttl < 0) ttl = sip->to ;
	if ((rs = uclustername_allocbegin(uip,sip->dt,ttl)) > 0) {
	    UCLUSTERNAME_A	uca ;
	    int			size = 0 ;
	    char		*bp ;

	    f = TRUE ;
	    memset(&uca,0,sizeof(UCLUSTERNAME_A)) ;
	    size += (strlen(sip->nn) + 1) ;
	    size += (strnlen(sip->rbuf,sip->rlen) + 1) ;
	    if ((rs = uc_libmalloc(size,&bp)) >= 0) {
	        uca.a = bp ;
	        aprev = uip->a ;
	        uca.nn = bp ;
	        bp = (strwcpy(bp,sip->nn,-1) + 1) ;
	        uca.cn = bp ;
	        bp = (strwcpy(bp,sip->rbuf,sip->rlen) + 1) ;
	        uca.et = sip->dt ;
	        uca.ttl = ttl ;
	    } /* end if (m-a) */

	    rs1 = uclustername_allocend(uip,&uca) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (alloc) */
	if ((rs >= 0) && (aprev != NULL)) uc_libfree(aprev) ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_cacheset) */


static int uclustername_allocbegin(UCLUSTERNAME *uip,time_t dt,int ttl)
{
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	if ((rs = uc_forklockbegin(-1)) >= 0) {
	    if ((rs = ptm_lock(&uip->m)) >= 0) {
	        if ((uip->a == NULL) || ((dt-uip->et) >= ttl)) {
	            if (! uip->f_allocget) {
	                uip->f_allocget = TRUE ;
	                f = TRUE ; /* indicate "got" */
	            }
	        } /* end if (need) */
	        rs1 = ptm_unlock(&uip->m) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = uc_forklockend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (forklock) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_allocbegin) */


static int uclustername_allocend(UCLUSTERNAME *uip,UCLUSTERNAME_A *ap)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if ((ap != NULL) && (ap->a != NULL)) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) {
	        if ((rs = ptm_lock(&uip->m)) >= 0) {
	            {
	                uip->f_allocget = FALSE ;
	                uip->a = ap->a ;
	                uip->nn = ap->nn ;
	                uip->cn = ap->cn ;
	                uip->et = ap->et ;
	                uip->ttl = ap->ttl ;
	            }
	            rs1 = ptm_unlock(&uip->m) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (mutex) */
	        rs1 = uc_forklockend() ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (forklock) */
	} /* end if (non-null) */
	return rs ;
}
/* end subroutine (subinfo_allocend) */


