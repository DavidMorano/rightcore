/* uc_isaexecve */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a Solaris® specific hack that is used to find a more optimized
        program to execute than the one that is the standard one. Some ISAs may
        have more optimized versions of some programs.


*******************************************************************************/


#undef	LOCAL_SOLARIS
#define	LOCAL_SOLARIS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	(1 * 60)	/* fairly long! */


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int uc_isaexecve(cchar *pfn,cchar **argv,cchar **envv)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;
	char *const *cav = (char *const *) argv ;
	char *const *cev = (char *const *) envv ;

	if (pfn == NULL) return SR_FAULT ;

	if (pfn[0] == '\0') return SR_INVALID ;

	repeat {

#if	LOCAL_SOLARIS
	    if ((rs = isaexec(pfn,cav,cev)) < 0) rs = (- errno) ;
#else
	    if ((rs = execve(pfn,cav,cev)) < 0) rs = (- errno) ;
#endif

	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
		    if (to_again-- > 0) {
	                msleep(1000) ;
	 	    } else {
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

	if (rs == SR_NOENT) {
	   rs = u_execve(pfn,argv,envv) ;
	}

	return rs ;
}
/* end subroutine (uc_isaexecve) */


