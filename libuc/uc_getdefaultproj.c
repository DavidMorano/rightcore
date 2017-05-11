/* uc_getdefaultproj */

/* interface component for UNIX® library-3c */
/* project DB access */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Now for some fun facts on the 'user_attr' and the 'project' databases
        Solaris access software (represented in part by this subroutine here).
        If the databases have any sort of corruption, there is a danger that the
        Solaris access software will memory-access-fault and cause a core dump
        -- as observed many times now. Care needs to taken that no entries in
        neither the 'user_attr' nor the 'project' databases are corrupted or
        malformed in any way. Until the crappy access software is fixed, core
        dumps are possibl if not guaranteed.

	Finaly note (if I may):

        I hate to say it, but debugging operating-system software without full
        ability to build or rebuild the OS itself is often quite difficult. And
        we can seldom actually patch up a binary OS even when we have and can
        correct the bugger source code involved.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<project.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	10


/* external subroutines */

extern int	msleep(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


#if	defined(SYSHAS_PROJECT) && (SYSHAS_PROJECT > 0)


int uc_getdefaultproj(name,pjp,rbuf,rlen)
const char	name[] ;
struct project	*pjp ;
char		rbuf[] ;
int		rlen ;
{
	struct project	*rp ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	if ((name == NULL) || (rbuf == NULL) || (pjp == NULL))
	    return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

	repeat {
	    rs = SR_OK ;
	    errno = 0 ;
	    rp = getdefaultproj(name,pjp,rbuf,(size_t) rlen) ;
	    if (rp == NULL) rs = ((errno == 0) ? SR_NOENT : (- errno)) ;
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

#if	CF_DEBUGS
	debugprintf("uc_getdefaultproj: ret rs=%d\n",rs) ;
	debugprintf("uc_getdefaultproj: proj=%s\n",pjp->pj_name) ;
#endif

	return rs ;
}
/* end subroutine (uc_getdefaultproj) */


#else /* defined(SYSHAS_PROJECT) && (SYSHAS_PROJECT > 0) */


int uc_getdefaultproj(name,pjp,rbuf,rlen)
const char	name[] ;
struct project	*pjp ;
char		rbuf[] ;
int		rlen ;
{

	return SR_NOSYS ;
}
/* end subroutine (uc_getdefaultproj) */


#if	(! defined(SUBROUTINE_GETPROJID)) || (SUBROUTINE_GETPROJID == 0)
#define	SUBROUTINE_GETPROJID	1
projid_t getprojid(void) { return 0 } ;
#endif /* (! defined(SUBROUTINE_GETPROJID)) || (SUBROUTINE_GETPROJID == 0) */


#if	(! defined(SUBROUTINE_GETTASKID)) || (SUBROUTINE_GETTASKID == 0)
#define	SUBROUTINE_GETTASKID	1
taskid_t gettaskid(void) { return 0 } ;
taskid_t settaskid(projid_t pjid,uint_t flags) { return 0 } ;
#endif /* (! defined(SUBROUTINE_GETTASKID)) || (SUBROUTINE_GETTASKID == 0) */


#endif /* defined(SYSHAS_PROJECT) && (SYSHAS_PROJECT > 0) */


