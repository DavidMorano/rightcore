/* uc_sigset */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<signal.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */


/* forward references */


/* exported subroutines */


int uc_sigsetempty(sigset_t *sp)
{
	int	rs ;
	if ((rs = sigemptyset(sp)) < 0) rs = (- errno) ;
	return rs ;
}
/* end subroutine (uc_sigsetempty) */


int uc_sigsetfill(sigset_t *sp)
{
	int	rs ;
	if ((rs = sigfillset(sp)) < 0) rs = (- errno) ;
	return rs ;
}
/* end subroutine (uc_sigsetfill) */


int uc_sigsetadd(sigset_t *sp,int sn)
{
	int	rs ;
	if ((rs = sigaddset(sp,sn)) < 0) rs = (- errno) ;
	return rs ;
}
/* end subroutine (uc_sigsetadd) */


int uc_sigsetdel(sigset_t *sp,int sn)
{
	int	rs ;
	if ((rs = sigdelset(sp,sn)) < 0) rs = (- errno) ;
	return rs ;
}
/* end subroutine (uc_sigsetdel) */


int uc_sigsetismem(sigset_t *sp,int sn)
{
	int	rs = SR_OK ;
	if (sn > 0) {
	    if ((rs = sigismember(sp,sn)) < 0) rs = (- errno) ;
	} else if (sn < 0)
	    rs = SR_INVALID ;
	return rs ;
}
/* end subroutine (uc_sigsetismem) */


int uc_sigemptyset(sigset_t *sp) {
	return uc_sigsetempty(sp) ;
}
/* end subroutine (uc_sigemptyset) */

int uc_sigfillset(sigset_t *sp) {
	return uc_sigsetfill(sp) ;
}
/* end subroutine (uc_sigfillset) */

int uc_sigaddset(sigset_t *sp,int sn) {
	return uc_sigsetadd(sp,sn) ;
}
/* end subroutine (uc_sigaddset) */

int uc_sigdelset(sigset_t *sp,int sn) {
	return uc_sigsetdel(sp,sn) ;
}
/* end subroutine (uc_sigdelset) */

int uc_sigismember(sigset_t *sp,int sn) {
	return uc_sigsetismem(sp,sn) ;
}
/* end subroutine (uc_sigismember) */


