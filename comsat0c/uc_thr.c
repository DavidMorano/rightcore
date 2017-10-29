/* uc_thr */

/* interface component for UNIX® library-3c */
/* UNIX® threads (specifically from Solaris) */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns the Current Working Directory (CWD).
	If you wanted the Present Working Directory (PWD), you should
	be calling 'getpwd()'.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<signal.h>
#include	<thread.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int uc_thrcreate(stackbase,stacksize,startfunc,arg,flags)
caddr_t		stackbase ;
size_t		stacksize ;
int		(*startfunc)(void *) ;
const void	*arg ;
long		flags ;
{
	thread_t	tid ;

	int	rs ;

	void	*targ = (void *) arg ;
	void	*(*tstart)(void *) = (void *(*)(void *)) startfunc ;


again:
	rs = thr_create(stackbase,stacksize,tstart,targ,flags,&tid) ;
	if (rs != 0) {
	    if (rs > 0) {
		rs = (- rs) ;
	    } else
		rs = SR_LIBACC ;
	} else
	    rs = (int) (tid & INT_MAX) ; /* we assume thread-ids can fit */

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrcreate) */


int uc_threxit(ex)
int		ex ;
{
	void	*stat = (void *) ex ;


	thr_exit(stat) ;

	return SR_OK ;
}
/* end subroutine (uc_threxit) */


int uc_thrjoin(tid,exp)
thread_t	tid ;
int		*exp ;
{
	thread_t	depart ;

	int	rs ;

	void	*stat ;


again:
	rs = thr_join(tid,&depart,&stat) ;
	if (rs > 0) {
	    rs = (- rs) ;
	} else {
	    if (exp != NULL) {
		long	v = (long) stat ;
		*exp = (int) (v & INT_MAX) ;
	    }
	    rs = (int) (depart & INT_MAX) ;
	}

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrjoin) */


int uc_thrsuspend(thread_t tid)
{
	int	rs ;


again:
	rs = thr_suspend(tid) ;
	if (rs > 0) {
	    rs = (- rs) ;
	} else if (rs < 0)
	    rs = SR_LIBACC ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrsuspend) */


int uc_thrcontinue(thread_t tid)
{
	int	rs ;


again:
	rs = thr_continue(tid) ;
	if (rs > 0) {
	    rs = (- rs) ;
	} else if (rs < 0)
	    rs = SR_LIBACC ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrcontinue) */


int uc_thrminstack()
{
	size_t	size ;

	int	rs ;


again:
	rs = SR_OK ;
	errno = 0 ;
	size = thr_min_stack() ;
	if (errno > 0) {
		rs = (- errno) ;
	} else
	    rs = (int) (size & INT_MAX) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrminstack) */


int uc_thrkill(tid,sig)
thread_t	tid ;
int		sig ;
{
	int	rs ;


again:
	rs  = thr_kill(tid,sig) ;
	if (rs > 0) {
	    rs = (- rs) ;
	} else if (rs < 0)
	    rs = SR_LIBACC ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrkill) */


int uc_thrmain()
{
	int	rs ;


again:
	errno = 0 ;
	rs = thr_main() ;
	if (rs < 0) {
	    if (errno != 0) {
		rs = (- errno) ;
	    } else
	        rs = SR_LIBACC ;
	}

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrmain) */


int uc_thrself()
{
	thread_t	tid ;

	int	rs ;


again:
	rs = SR_OK ;
	errno = 0 ;
	tid = thr_self() ;
	if (errno != 0) {
	    rs = (- errno) ;
	} else
	    rs = (int) (tid & INT_MAX) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrself) */


int uc_thryield()
{
	int	rs ;


again:
	rs = SR_OK ;
	errno = 0 ;
	thr_yield() ;
	if (errno != 0) rs = (- errno) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thryield) */


int uc_thrsigsetmask(int how,const sigset_t *nsp,sigset_t *osp)
{
	int	rs = SR_OK ;


again:
	errno = 0 ;
	rs = thr_sigsetmask(how,nsp,osp) ;
	if (rs != 0) {
	    if (rs > 0) {
	        rs = (- rs) ;
	    } else if (errno != 0)
	        rs = (- errno) ;
	}

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrsigsetmask) */


int uc_thrstksegment(stack_t *stk)
{
	int	rs ;


again:
	rs = thr_stksegment(stk) ;
	if (rs > 0) {
	    rs = (- rs) ;
	} else if (rs < 0)
	    rs = SR_LIBACC ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrstksegment) */


int uc_thrkeycreate(thread_key_t *keyp,void (*destructor)(void *))
{
	int	rs ;


again:
	rs = thr_keycreate(keyp,destructor) ;
	if (rs > 0) {
	    rs = (- rs) ;
	} else if (rs < 0)
	    rs = SR_LIBACC ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrkeycreate) */


int uc_thrsetspecific(thread_key_t key,void *value)
{
	int	rs ;


again:
	rs = thr_setspecific(key,value) ;
	if (rs > 0) {
	    rs = (- rs) ;
	} else if (rs < 0)
	    rs = SR_LIBACC ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrsetspecific) */


int uc_thrgetspecific(thread_key_t key,void **value)
{
	int	rs ;


again:
	rs = thr_getspecific(key,value) ;
	if (rs > 0) {
	    rs = (- rs) ;
	} else if (rs < 0)
	    rs = SR_LIBACC ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrgetspecific) */


int uc_thrgetconcurrency()
{
	int	rs ;


again:
	errno = 0 ;
	rs = thr_getconcurrency() ;
	if (errno != 0) rs = (- errno) ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrgetconcurrency) */


int uc_thrsetconcurrency(new)
int		new ;
{
	int	rs ;


again:
	rs = thr_setconcurrency(new) ;
	if (rs > 0) {
	    rs = (- rs) ;
	} else if (rs < 0)
	    rs = SR_LIBACC ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (uc_thrsetconcurrency) */


