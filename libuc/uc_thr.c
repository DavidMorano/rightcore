/* uc_thr */

/* interface component for UNIX® library-3c */
/* UNIX® threads (specifically from Solaris) */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code module provided a cleaned up interface to the native Solaris®
	thread facility.

	Generally, this interface is ignored in favor of the POSIX® interface.


	= Thread creation.

	Synopsis:

	int uc_thrcreate(sbase,ssize,func,arg,tfl)
	caddr_t		sbase ;
	size_t		ssize ;
	thrfunc_t	func ;
	cvoid		*arg ;
	long		tfl ;

	Arguments:

	sbase		stack base address
	ssize		statk size
	func		function to call as a thread
	arg		argument to thread function
	tfl		flags to manage thread creation

	Returns:

	<0		error
	>=0		thread ID


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


/* local defines */

#ifndef	TYPEDEF_CVOID
#define	TYPEDEF_CVOID	1
typedef const void	cvoid ;
#endif


/* typedefs */

typedef int (*thrfunc_t)(void *) ;


/* external subroutines */


/* exported subroutines */


int uc_thrcreate(caddr_t sbase,size_t ssize,thrfunc_t func,cvoid *arg,long tfl)
{
	thread_t	tid ;
	int		rs ;
	void		*targ = (void *) arg ;
	void		*(*tstart)(void *) = (void *(*)(void *)) func ;

	repeat {
	    rs = thr_create(sbase,ssize,tstart,targ,tfl,&tid) ;
	    if (rs != 0) {
	        if (rs > 0) {
		    rs = (- rs) ;
	        } else
		    rs = SR_LIBACC ;
	    } else {
	        rs = (int) (tid & INT_MAX) ; /* we assume thread-ids can fit */
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrcreate) */


int uc_threxit(int ex)
{
	void		*stat = (void *) ex ;
	thr_exit(stat) ;
	return SR_OK ;
}
/* end subroutine (uc_threxit) */


int uc_thrjoin(thread_t tid,int *exp)
{
	thread_t	depart ;
	int		rs ;
	void		*stat ;

	repeat {
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
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrjoin) */


int uc_thrsuspend(thread_t tid)
{
	int		rs ;

	repeat {
	    rs = thr_suspend(tid) ;
	    if (rs > 0) {
	        rs = (- rs) ;
	    } else if (rs < 0) {
	        rs = SR_LIBACC ;
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrsuspend) */


int uc_thrcontinue(thread_t tid)
{
	int		rs ;

	repeat {
	    rs = thr_continue(tid) ;
	    if (rs > 0) {
	        rs = (- rs) ;
	    } else if (rs < 0) {
	        rs = SR_LIBACC ;
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrcontinue) */


int uc_thrminstack()
{
	size_t		size ;
	int		rs ;

	repeat {
	     rs = SR_OK ;
	     errno = 0 ;
	     size = thr_min_stack() ;
	     if (errno > 0) {
		rs = (- errno) ;
	     } else {
	         rs = (int) (size & INT_MAX) ;
	     }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrminstack) */


int uc_thrkill(thread_t tid,int sig)
{
	int		rs ;

	repeat {
	    rs  = thr_kill(tid,sig) ;
	    if (rs > 0) {
	        rs = (- rs) ;
	    } else if (rs < 0) {
	        rs = SR_LIBACC ;
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrkill) */


int uc_thrmain()
{
	int		rs ;

	repeat {
	    errno = 0 ;
	    rs = thr_main() ;
	    if (rs < 0) {
	        if (errno != 0) {
		    rs = (- errno) ;
	        } else {
	            rs = SR_LIBACC ;
		}
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrmain) */


int uc_thrself()
{
	thread_t	tid ;
	int		rs ;

	repeat {
	    rs = SR_OK ;
	    errno = 0 ;
	    tid = thr_self() ;
	    if (errno != 0) {
	        rs = (- errno) ;
	    } else {
	        rs = (int) (tid & INT_MAX) ;
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrself) */


int uc_thryield()
{
	int		rs ;

	repeat {
	    rs = SR_OK ;
	    errno = 0 ;
	    thr_yield() ;
	    if (errno != 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thryield) */


int uc_thrsigsetmask(int how,const sigset_t *nsp,sigset_t *osp)
{
	int		rs ;

	repeat {
	    errno = 0 ;
	    rs = thr_sigsetmask(how,nsp,osp) ;
	    if (rs != 0) {
	        if (rs > 0) {
	            rs = (- rs) ;
	        } else if (errno != 0) {
	            rs = (- errno) ;
		}
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrsigsetmask) */


int uc_thrstksegment(stack_t *stk)
{
	int		rs ;

	repeat {
	    rs = thr_stksegment(stk) ;
	    if (rs > 0) {
	        rs = (- rs) ;
	    } else if (rs < 0) {
	        rs = SR_LIBACC ;
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrstksegment) */


int uc_thrkeycreate(thread_key_t *keyp,void (*destructor)(void *))
{
	int		rs ;

	repeat {
	    rs = thr_keycreate(keyp,destructor) ;
	    if (rs > 0) {
	        rs = (- rs) ;
	    } else if (rs < 0) {
	        rs = SR_LIBACC ;
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrkeycreate) */


int uc_thrsetspecific(thread_key_t key,void *value)
{
	int		rs ;

	repeat {
	    rs = thr_setspecific(key,value) ;
	    if (rs > 0) {
	        rs = (- rs) ;
	    } else if (rs < 0) {
	        rs = SR_LIBACC ;
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrsetspecific) */


int uc_thrgetspecific(thread_key_t key,void **value)
{
	int		rs ;

	repeat {
	    rs = thr_getspecific(key,value) ;
	    if (rs > 0) {
	        rs = (- rs) ;
	    } else if (rs < 0) {
	        rs = SR_LIBACC ;
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrgetspecific) */


int uc_thrgetconcurrency()
{
	int		rs ;

	repeat {
	    errno = 0 ;
	    rs = thr_getconcurrency() ;
	    if (errno != 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrgetconcurrency) */


int uc_thrsetconcurrency(int new)
{
	int		rs ;

	repeat {
	    rs = thr_setconcurrency(new) ;
	    if (rs > 0) {
	        rs = (- rs) ;
	    } else if (rs < 0) {
	        rs = SR_LIBACC ;
	    }
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_thrsetconcurrency) */


