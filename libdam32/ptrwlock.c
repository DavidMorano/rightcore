/* ptrwlock */
/* ** broken on SOLARIS®! ** */

/* POSIX® Thread Read-Weite Lock (PRWLOCK) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-07-23, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	** broken on SOLARIS®! **

	This module provides a sanitized version of the standard POSIX®
	semaphore facility provided with some new UNIX®i.  Some operating
	system problems are managed within these routines for the common stuff
	that happens when a poorly configured OS gets overloaded!

	Enjoy!

	Important note:

	** broken on SOLARIS®! **

	Use a working read-write lock, like for example LOCKRW by
	David A­D­ Morano.

	Updated note:

	This object (or rather the underlying RW-LOCK facility itself) is
	**broken** on Solaris®.  It is rather complicated but when this object
	is used within a shared object that was loaded by a program manually
	(through |dlopen(3dl)|) it fails by messing up its lock state for some
	reason.  This failure, although quite obscure for most, makes this
	object unusable for any of our purposes.  Use our own read-write lock
	(which we developed ourselves) instead.  We do not make stupid-ass
	mistakes like the Solaris® developers do.  Maybe we make our own types
	of mistakes, but not the stupid-ass mistakes that the Solaris®
	developers have done.


*******************************************************************************/


#define	PRWLOCK_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<pthread.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ptrwlock.h"
#include	"ptrwa.h"


/* local defines */

#undef	NLPS
#define	NLPS		2


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	msleep(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int		ptrwlock_create(PTRWLOCK *,PTRWA *) ;


/* local variables */


/* exported subroutines */


int ptrwlock_create(PTRWLOCK *psp,PTRWA *atp)
{
	int		rs ;

	if (psp == NULL) return SR_FAULT ;

	repeat {
	    if ((rs = pthread_rwlock_init(psp,atp)) > 0) rs = (- rs) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (ptrwlock_create) */


int ptrwlock_destroy(PTRWLOCK *psp)
{
	int		rs ;

	if (psp == NULL) return SR_FAULT ;

	repeat {
	    if ((rs = pthread_rwlock_destroy(psp)) > 0) rs = (- rs) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (ptrwlock_destroy) */


int ptrwlock_rdlock(PTRWLOCK *psp)
{
	int		rs ;

	if (psp == NULL) return SR_FAULT ;

	repeat {
	    if ((rs = pthread_rwlock_rdlock(psp)) > 0) rs = (- rs) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (ptrwlock_rdlock) */


int ptrwlock_tryrdlock(PTRWLOCK *psp)
{
	int		rs ;

	if (psp == NULL) return SR_FAULT ;

	repeat {
	    if ((rs = pthread_rwlock_tryrdlock(psp)) > 0) rs = (- rs) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (ptrwlock_tryrdlock) */


int ptrwlock_rdlockto(PTRWLOCK *psp,int to)
{
	const int	mint = (1000/NLPS) ;
	int		rs ;
	int		cto ;
	int		c = 0 ;
	int		f_exit = FALSE ;

	if (psp == NULL) return SR_FAULT ;

	if (to < 0) to = (INT_MAX/(2*NLPS)) ;
	cto = (to*NLPS) ;

	repeat {
	    if ((rs = pthread_rwlock_tryrdlock(psp)) > 0) rs = (- rs) ;
	    if (rs < 0) {
		switch (rs) {
		case SR_BUSY:
		    if (++c < cto) {
	    		msleep(mint) ;
		    } else {
			f_exit = FALSE ;
		    }
		    break ;
		case SR_INTR:
		    break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end siwtch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (ptrwlock_rdlockto) */


int ptrwlock_wrlock(PTRWLOCK *psp)
{
	int		rs ;

	if (psp == NULL) return SR_FAULT ;

	repeat {
	    if ((rs = pthread_rwlock_wrlock(psp)) > 0) rs = (- rs) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (ptrwlock_wrlock) */


int ptrwlock_trywrlock(PTRWLOCK *psp)
{
	int		rs ;

	if (psp == NULL) return SR_FAULT ;

	repeat {
	    if ((rs = pthread_rwlock_trywrlock(psp)) > 0) rs = (- rs) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (ptrwlock_trywrlock) */


int ptrwlock_wrlockto(PTRWLOCK *psp,int to)
{
	const int	mint = (1000/NLPS) ;
	int		rs ;
	int		cto ;
	int		c = 0 ;
	int		f_exit = FALSE ;

	if (psp == NULL) return SR_FAULT ;

	if (to < 0) to = (INT_MAX/(2*NLPS)) ;
	cto = (to*NLPS) ;

	repeat {
	    if ((rs = pthread_rwlock_trywrlock(psp)) > 0) rs = (- rs) ;
	    if (rs < 0) {
		switch (rs) {
		case SR_BUSY:
		    if (++c < cto) {
	    	        msleep(mint) ;
		    } else {
	    	        f_exit = TRUE ;
		    }
		    break ;
		default:
		    f_exit = TRUE ;
		    break ;
		} /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (ptrwlock_wrlockto) */


int ptrwlock_unlock(PTRWLOCK *psp)
{
	int		rs ;

	if (psp == NULL) return SR_FAULT ;

	repeat {
	    if ((rs = pthread_rwlock_unlock(psp)) > 0) rs = (- rs) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (ptrwlock_unlock) */


