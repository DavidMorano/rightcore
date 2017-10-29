/* uc_clustername */

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

	int uc_clusternameget(rbuf,rlen,nodename)
	char		rbuf[] ;
	int		rlen ;
	const char	nodename[] ;

	Arguments:

	rbuf		buffer to receive the requested cluster name
	rlen		length of supplied buffer
	nodename	nodename used to find associated cluster

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

	Q. Was the amount of complication supported by the need?
	A. Looking at it now ... maybe not.

	Q. Were there any alternatives?
	A. Yes; the predicessor to this present implementation was an 
	   implementation that was quite simple, but it had a lot of static
	   memory storage.  It was the desire to eliminate the static memory
	   storage that led to this present implementation.

	Q. Are there ways to clean this up further?
	A. Probably, but it looks I have already done more to this simple
	   function than may have been ever warranted to begin with!

	Q. Did this subroutine have to be Asyc-Signal-Safe?
	A. Not really.

	Q. Then why did you do it?
	A. The system-call |uname(2)| is Async-Signal-Safe.  Since this
	   subroutine (|getclustername(3dam)|) sort of looks like
	   |uname(2)| (of a sort), I thought it was a good idea.

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

#define	CLUSTERNAME	struct clustername

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
	const char	*nn ;
	const char	*cn ;
	char		*a ;		/* memory-allocation */
	volatile int	f_init ; 	/* race-condition, blah, blah */
	volatile int	f_initdone ;
} ;

struct subinfo {
	const char	*nn ;
	char		*rbuf ;
	time_t		dt ;
	int		rlen ;
	int		to ;
} ;


/* forward references */

void		clustername_fini() ;

static void	clustername_atforkbefore() ;
static void	clustername_atforkafter() ;

static int	clustername_end(CLUSTERNAME *) ;

static int subinfo_start(SUBINFO *,char *,int,const char *) ;
static int subinfo_finish(SUBINFO *) ;
static int subinfo_cacheget(SUBINFO *,CLUSTERNAME *) ;
static int subinfo_cacheset(SUBINFO *,CLUSTERNAME *) ;


/* local variables */

static CLUSTERNAME	clustername_data ; /* zero-initialized */


/* exported subroutines */


int clustername_init()
{
	CLUSTERNAME	*uip = &clustername_data ;
	int	rs = 1 ;
	if (! uip->f_init) {
	    void	(*b)() = clustername_atforkbefore ;
	    void	(*a)() = clustername_atforkafter ;
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(clustername_fini)) >= 0) {
	                rs = 0 ;
	                uip->f_initdone = TRUE ;
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
	    while (! uip->f_initdone) msleep(1) ;
	}
	return rs ;
}
/* end subroutine (clustername_init) */


void clustername_fini()
{
	CLUSTERNAME	*uip = &clustername_data ;
	if (uip->f_initdone) {
	    {
	 	clustername_end(uip) ;
	    }
	    {
	        void	(*b)() = clustername_atforkbefore ;
	        void	(*a)() = clustername_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(CLUSTERNAME)) ;
	} /* end if (was initialized) */
}
/* end subroutine (clustername_fini) */


int uc_clusternameget(rbuf,rlen,nn)
char		rbuf[] ;
int		rlen ;
const char	nn[] ;
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
	    if ((rs = clustername_init()) >= 0) {
		SUBINFO		si ;
		if ((rs = subinfo_start(&si,rbuf,rlen,nn)) >= 0) {
	            CLUSTERNAME	*uip = &clustername_data ;
		    rs = subinfo_cacheget(&si,uip) ;
		    len = rs ;
		    rs1 = subinfo_finish(&si) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (subinfo) */
	    } /* end if (init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_clusternameget) */


int uc_clusternameset(cbuf,clen,nn)
const char	cbuf[] ;
int		clen ;
const char	nn[] ;
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;

	if (cbuf == NULL) return SR_FAULT ;
	if (nn == NULL) return SR_FAULT ;

	if (cbuf[0] == '\0') return SR_INVALID ;
	if (nn[0] == '\0') return SR_INVALID ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = clustername_init()) >= 0) {
		SUBINFO		si ;
		if ((rs = subinfo_start(&si,(char *) cbuf,clen,nn)) >= 0) {
	            CLUSTERNAME	*uip = &clustername_data ;
		    rs = subinfo_cacheset(&si,uip) ;
		    rs1 = subinfo_finish(&si) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (subinfo) */
	    } /* end if (init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

	return rs ;
}
/* end subroutine (uc_clusternameget) */


/* local subroutines */


static int subinfo_start(sip,rbuf,rlen,nn)
SUBINFO		*sip ;
char		rbuf[] ;
int		rlen ;
const char	nn[] ;
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


static int subinfo_cacheget(SUBINFO *sip,CLUSTERNAME *uip)
{
	const int	to = sip->to ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if ((rs = uc_forklockbegin(-1)) >= 0) {
	    if ((rs = ptm_lock(&uip->m)) >= 0) {
		if (uip->a != NULL) {
	            if ((uip->et > 0) && ((sip->dt-uip->et) < to)) {
	                    if (strcmp(sip->nn,uip->nn) == 0) {
	    	                rs = sncpy1(sip->rbuf,sip->rlen,uip->cn) ;
				len = rs ;
		            }
	            } /* end if (not timed-out) */
		} /* end if (loaded) */
		rs1 = ptm_unlock(&uip->m) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = uc_forklockend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (forklock) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getsubinfo_cache) */


static int subinfo_cacheset(SUBINFO *sip,CLUSTERNAME *uip)
{
	const int	to = sip->to ;
	int		rs ;
	int		rs1 ;
	if ((rs = uc_forklockbegin(-1)) >= 0) {
	    if ((rs = ptm_lock(&uip->m)) >= 0) {
	       if ((uip->a == NULL) || ((sip->dt-uip->et) >= to)) {
	            int		size = 0 ;
	            char	*bp ;
		    if (uip->a != NULL) {
			uc_libfree(uip->a) ;
			uip->a = NULL ;
		    }
	            size += (strlen(sip->nn) + 1) ;
	            size += (strlen(sip->rbuf) + 1) ;
	            if ((rs = uc_libmalloc(size,&bp)) >= 0) {
			uip->a = bp ;
	                uip->nn = bp ;
	                bp = (strwcpy(bp,sip->nn,-1) + 1) ;
	                uip->cn = bp ;
	                bp = (strwcpy(bp,sip->rbuf,-1) + 1) ;
			uip->et = sip->dt ;
	            } /* end if (memory-allocation) */
	        } /* end if (need to get information) */
		rs1 = ptm_unlock(&uip->m) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = uc_forklockend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (forklock) */
	return rs ;
}
/* end subroutine (subinfo_cacheset) */


static int clustername_end(CLUSTERNAME *uip)
{
	int	rs = SR_OK ;
	int	rs1 ;
	if (uip->a != NULL) {
	    rs1 = uc_libfree(uip->a) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->a = NULL ;
	    uip->nn = NULL ;
	    uip->cn = NULL ;
	}
	return rs ;
}
/* end subroutine (clustername_end) */


static void clustername_atforkbefore()
{
	CLUSTERNAME	*uip = &clustername_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (clustername_atforkbefore) */


static void clustername_atforkafter()
{
	CLUSTERNAME	*uip = &clustername_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (clustername_atforkafter) */


