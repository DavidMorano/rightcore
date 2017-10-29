/* upt */

/* UNIX® POSIX Thread manipulation */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-20, David A­D­ Morano

	This is a complete rewrite of the trash that performed this
	function previously.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module provides an informal way to abstract some of the junk needed
        to work with threads.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>
#include	<signal.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<pta.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOMEM	5

#ifndef	VARNCPU
#define	VARNCPU		"NCPU"
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	msleep(int) ;


/* local structures */

struct ourarg {
	void	*ap ;
	int	(*start)(void *) ;
} ;


/* forward references */

static int	uptcreator(pthread_t *,pthread_attr_t *,void *) ;

static void	*uptruner(void *) ;


/* exported subroutines */


int uptcreate(rp,ptap,start,arg)
pthread_t	*rp ;
pthread_attr_t	*ptap ;
int		(*start)(void *) ;
void		*arg ;
{
	struct ourarg	*oap ;
	const int	size = sizeof(struct ourarg) ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("uptcreate: ent\n") ;
#endif

	if ((start == NULL) || (rp == NULL))
	    return SR_FAULT ;

	if ((rs = uc_libmalloc(size,&oap)) >= 0) {
	    sigset_t	nsm, osm ;
	    oap->ap = arg ;
	    oap->start = start ;
	    uc_sigsetfill(&nsm) ;
	    if ((rs = pt_sigmask(SIG_BLOCK,&nsm,&osm)) >= 0) {

		rs = uptcreator(rp,ptap,oap) ;

		pt_sigmask(SIG_SETMASK,&osm,NULL) ;
	    } /* end if (sigblock) */
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (uptcreate) */


int uptexit(int rs)
{
	void	*vrp ;

	vrp = (void *) rs ;
	pthread_exit(vrp) ;

	return SR_OK ;
}
/* end subroutine (uptexit) */


int uptonce(op,initsub)
pthread_once_t	*op ;
void		(*initsub)() ;
{
	int	rs ;

again:
	rs = pthread_once(op,initsub) ;
	if (rs > 0)
	    rs = (- rs) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uptonce) */


int uptjoin(pthread_t tid,int *rsp)
{
	void	*vp, **vrpp ;
	int	rs ;

	vrpp = (rsp != NULL) ? &vp : NULL ;

again:
	rs = pthread_join(tid,vrpp) ;
	if (rs > 0) rs = (- rs) ;

	if (rs == SR_INTR) goto again ;

	if (rsp != NULL)
	    *rsp = (int) vp ;

	return rs ;
}
/* end subroutine (uptjoin) */


int uptdetach(tid)
pthread_t	tid ;
{
	int	rs ;

again:
	rs = pthread_detach(tid) ;
	if (rs > 0)
	    rs = (- rs) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uptdetach) */


int uptcancel(tid)
pthread_t	tid ;
{
	int	rs ;

again:
	rs = pthread_cancel(tid) ;
	if (rs > 0)
	    rs = (- rs) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uptcancel) */


int uptsetschedparam(tid,policy,pp)
pthread_t	tid ;
int		policy ;
struct sched_param	*pp ;
{
	int	rs ;

again:
	rs = pthread_setschedparam(tid,policy,pp) ;
	if (rs > 0)
	    rs = (- rs) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uptsetschedparam) */


int uptgetschedparam(tid,policyp,pp)
pthread_t	tid ;
int		*policyp ;
struct sched_param	*pp ;
{
	int	rs ;

again:
	rs = pthread_getschedparam(tid,policyp,pp) ;
	if (rs > 0)
	    rs = (- rs) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uptgetschedparam) */


int uptsetcancelstate(intstate,oldstate)
int		intstate ;
int		*oldstate ;
{
	int	rs ;

again:
	rs = pthread_setcancelstate(intstate,oldstate) ;
	if (rs > 0)
	    rs = (- rs) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uptsetcancelstate) */


int uptsetcanceltype(intstate,oldstate)
int		intstate ;
int		*oldstate ;
{
	int	rs ;

again:
	rs = pthread_setcanceltype(intstate,oldstate) ;
	if (rs > 0)
	    rs = (- rs) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uptsetcanceltype) */


int upttestcancel()
{

	pthread_testcancel() ;

	return SR_OK ;
}
/* end subroutine (upttestcancel) */


int uptequal(t1,t2)
pthread_t	t1, t2 ;
{
	int	rs ;

	rs = pthread_equal(t1,t2) ;

	return rs ;
}
/* end subroutine (uptequal) */


int uptself(rp)
pthread_t	*rp ;
{
	pthread_t	tid ;
	int		rc ;

	tid = pthread_self() ;

	if (rp != NULL) *rp = tid ;

	rc = (tid & INT_MAX) ;
	return rc ;
}
/* end subroutine (uptself) */


int uptgetconcurrency()
{
	int	rs ;
	int	c = 0 ;

again:
	errno = 0 ;
	rs = pthread_getconcurrency() ;
	c = rs ;
	if (errno != 0) rs = (- errno) ;

	if (rs == SR_INTR) goto again ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (uptgetconcurrency) */


int uptsetconcurrency(c)
int	c ;
{
	int	rs ;

	if (c < 1) c = 1 ;

again:
	rs = pthread_setconcurrency(c) ;
	if (rs > 0) rs = (- rs) ;

	if (rs == SR_INTR) goto again ;

	return c ;
}
/* end subroutine (uptsetconcurrency) */


int uptatfork(pre,par,chi)
void	(*pre)() ;
void	(*par)() ;
void	(*chi)() ;
{
	int	rs ;

again:
	rs = pthread_atfork(pre,par,chi) ;
	if (rs != 0) rs = (- errno) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uptatfork) */


/* ARGSUSED */
int uptncpus(int w)
{
	int		rs1 ;
	int		n = 0 ;
	const char	*vn = VARNCPU ;
	const char	*cp ;

	if ((n == 0) && ((cp = getenv(vn)) != NULL)) {
	    int	v ;
	    rs1 = cfdeci(cp,-1,&v) ;
	    if (rs1 >= 0) n = v ;
	} /* end if (environment) */

#ifdef	_SC_NPROCESSORS_ONLN
	if (n == 0) {
	    rs1 = uc_nprocessors(w) ;
	    if (rs1 >= 0) n = rs1 ;
	}
#endif /* _SC_NPROCESSORS_ONLN */

	if (n == 0) n = 1 ;

	return n ;
}
/* end subroutine (uptncpus) */


/* local subroutines */


static int uptcreator(rp,ptap,arg)
pthread_t	*rp ;
pthread_attr_t	*ptap ;
void		*arg ;
{
	int	to_nomem = TO_NOMEM ;
	int	rs ;

again:
	if ((rs = pthread_create(rp,ptap,uptruner,arg)) > 0) rs = (- rs) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_NOMEM:
	        if (to_nomem-- > 0) {
		    msleep(1000) ;
	            goto again ;
		}
	        break ;
	    case SR_INTR:
		goto again ;
	    } /* end switch */
	} /* end if (error) */

	return rs ;
}
/* end subroutine (uptcreator) */


static void *uptruner(void *vp)
{
	struct ourarg	*oap = (struct ourarg *) vp ;
	void		*vrp ;
	int		rs ;

	if (oap != NULL) {
	    int		(*start)(void *) = oap->start ;
	    void	*arg = oap->ap ;

	    uc_libfree(oap) ;

	    rs = (*start)(arg) ;
	} else
	    rs = SR_NOEXEC ;

	vrp = (void *) rs ;
	return vrp ;
}
/* end subroutine (uptruner) */


