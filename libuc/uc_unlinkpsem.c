/* uc_unlinkpsem */

/* interface component for UNIX® library-3c */
/* POSIX semaphore ("sem(3rt)') file-unlink */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<semaphore.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


#if	defined(SYSHAS_PSEM) && (SYSHAS_PSEM > 0)

int uc_unlinkpsem(cchar *fname)
{
	int		rs ;

	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

	repeat {
	    errno = 0 ;
	    if ((rs = sem_unlink(fname)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_unlinkpsem) */

#else /* defined(SYSHAS_PSEM) && (SYSHAS_PSEM > 0) */

int uc_unlinkpsem(cchar *fname)
{
	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;
	return SR_NOSYS ;
}
/* end subroutine (uc_unlinkpsem) */

#endif /* defined(SYSHAS_PSEM) && (SYSHAS_PSEM > 0) */


