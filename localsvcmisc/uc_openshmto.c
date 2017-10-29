/* uc_openshmto */

/* interface component for UNIX® library-3c */
/* open POSIX share-memory (w/ time-out) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine opens a POSIX® shared-memory ('sem(3rt)') object but
        with a time-out. What does this mean to have a time-out while trying to
        open a shared memory segment? It means that if the segment is access
        protected, we continue to try until the time-out value has expired.

	Synopsis:

	int uc_openshmto(const char *shmname,int of,mode_t om,int to)

	Arguments:

	shmname		string representing the name of the shared memory
	of		open flags
	om		open mode
	to		time-out

	Returns:

	<0		error
	>=0		OK (file-descriptor)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	msleep(int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


#if	defined(SYSHAS_PSHM) && (SYSHAS_PSHM > 0)

int uc_openshmto(const char *shmname,int of,mode_t om,int to)
{
	int		rs = SR_OK ;
	int		i ;
	int		fd = -1 ;

	if (to < 0) to = INT_MAX ;

	for (i = 0 ; to-- > 0 ; i += 1) {
	    if (i > 0) msleep(1000) ;
	    rs = uc_openshm(shmname,of,om) ;
	    fd = rs ;
	    if ((rs >= 0) || (rs != SR_ACCESS)) break ;
	} /* end while */

	if ((rs < 0) && (to == 0)) rs = SR_TIMEDOUT ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (uc_openshmto) */

#else /* defined(SYSHAS_PSHM) && (SYSHAS_PSHM > 0) */

/* ARGSUSED */
int uc_openshmto(const char *fname,int of,mode_t om,int to)
{
	return SR_NOSYS ;
}
/* end subroutine (uc_openshm) */

#endif /* defined(SYSHAS_PSHM) && (SYSHAS_PSHM > 0) */


