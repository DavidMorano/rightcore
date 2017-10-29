/* psem */

/* Posix Semaphore (PSEM) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_CONDUNLINK	1		/* conditional unlink */


/* revision history:

	= 1999-07-23, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module provides a sanitized version of the standard POSIX®
        semaphore facility provided with some new UNIX®i. Some operating system
        problems are managed within these routines for the common stuff that
        happens when a poorly configured OS gets overloaded!

	Enjoy!


*******************************************************************************/


#define	PSEM_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<semaphore.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"psem.h"


/* local defines */

#undef	NLPS
#define	NLPS		5	/* number of polls per second */


/* external subroutines */

extern int	msleep(int) ;


/* forward references */

int		psem_create(PSEM *,int,uint) ;


/* local variables */


/* exported subroutines */


int psem_create(PSEM *psp,int pshared,uint count)
{
	const int	ic = (int) (count & INT_MAX) ;
	int		rs ;
	if (psp == NULL) return SR_FAULT ;
	repeat {
	    if ((rs = sem_init(psp,pshared,ic)) < 0) {
		rs = (- errno) ;
	    }
	} until (rs != SR_INTR) ;
	return rs ;
}
/* end subroutine (psem_create) */


int psem_destroy(PSEM *psp)
{
	int		rs ;
	if (psp == NULL) return SR_FAULT ;
	repeat {
	    if ((rs = sem_destroy(psp)) < 0) {
		rs = (- errno) ;
	    }
	} until (rs != SR_INTR) ;
	return rs ;
}
/* end subroutine (psem_destroy) */


int psem_count(PSEM *psp)
{
	return psem_getvalue(psp,NULL) ;
}
/* end subroutine (psem_count) */


int psem_getvalue(PSEM *psp,int *rp)
{
	int		rs ;
	int		c = 0 ;
	if (psp == NULL) return SR_FAULT ;
	if (rp == NULL) rp = &c ;
	repeat {
	    if ((rs = sem_getvalue(psp,rp)) < 0) {
		rs = (- errno) ;
	    } else {
		rs = *rp ;
	    }
	} until (rs != SR_INTR) ;
	return rs ;
}
/* end subroutine (psem_getvalue) */


int psem_waiti(PSEM *psp)
{
	int		rs ;
	if (psp == NULL) return SR_FAULT ;
	repeat {
	    if ((rs = sem_wait(psp)) < 0) {
		rs = (- errno) ;
	    }
	} until (rs != SR_INTR) ;
	return rs ;
}
/* end subroutine (psem_waiti) */


int psem_wait(PSEM *psp)
{
	int		rs ;
	if (psp == NULL) return SR_FAULT ;
	repeat {
	    if ((rs = sem_wait(psp)) < 0) {
		rs = (- errno) ;
	    }
	} until (rs != SR_INTR) ;
	return rs ;
}
/* end subroutine (psem_wait) */


int psem_trywait(PSEM *psp)
{
	int		rs ;
	if (psp == NULL) return SR_FAULT ;
	repeat {
	    if ((rs = sem_trywait(psp)) < 0) {
		rs = (- errno) ;
	    }
	} until (rs != SR_INTR) ;
	return rs ;
}
/* end subroutine (psem_trywait) */


int psem_waiter(PSEM *psp,int to)
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
	    if ((rs = sem_trywait(psp)) < 0) rs = (- errno) ;
	    if (rs < 0) {
		switch (rs) {
	    	case SR_AGAIN:
		    if (c++ < cto) {
			msleep(mint) ;
		    } else {
			rs = SR_TIMEDOUT ;
			f_exit = TRUE ;
		    }
		    break ;
		case SR_INTR:
		    break ;
		default:
		    f_exit = TRUE ;
		    break ;
		} /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;
	return rs ;
}
/* end subroutine (psem_waiter) */


int psem_post(PSEM *psp)
{
	int		rs ;
	if (psp == NULL) return SR_FAULT ;
	repeat {
	    if ((rs = sem_post(psp)) < 0) {
		rs = (- errno) ;
	    }
	} until (rs != SR_INTR) ;
	return rs ;
}
/* end subroutine (psem_post) */


int psem_close(PSEM *psp)
{
	int		rs ;
	if (psp == NULL) return SR_FAULT ;
	repeat {
	    if ((rs = sem_close(psp)) < 0) {
		rs = (- errno) ;
	    }
	} until (rs != SR_INTR) ;
	return rs ;
}
/* end subroutine (psem_close) */


