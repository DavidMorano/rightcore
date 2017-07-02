/* sigblock */

/* block process signals */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This was created along with the DATE object.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This small object provides a way to block (and unblock) process signals.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"sigblock.h"


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* exported variables */


/* local subroutines */


/* forward references */


/* local variables */


/* exported subroutines */


int sigblock_start(SIGBLOCK *op,const int *sigs)
{
	sigset_t	nsm ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (sigs != NULL) {
	    int	i ;
	    uc_sigsetempty(&nsm) ;
	    for (i = 0 ; (rs >= 0) && (sigs[i] > 0) ; i += 1) {
	        rs = uc_sigsetadd(&nsm,sigs[i]) ;
	    }
	} else {
	    rs = uc_sigsetfill(&nsm) ;
	}

	if (rs >= 0) {
#if	defined(PTHREAD) && (PTHREAD > 0)
	    rs = pt_sigmask(SIG_BLOCK,&nsm,&op->osm) ;
#else
	    rs = u_sigprocmask(SIG_BLOCK,&nsm,&op->osm) ;
#endif /* defined(PTHREAD) && (PTHREAD > 0) */
	} /* end if */

	return rs ;
}
/* end subroutine (sigblock_start) */


int sigblock_finish(SIGBLOCK *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	defined(PTHREAD) && (PTHREAD > 0)
	rs = pt_sigmask(SIG_SETMASK,&op->osm,NULL) ;
#else
	rs = u_sigprocmask(SIG_SETMASK,&op->osm,NULL) ;
#endif /* defined(PTHREAD) && (PTHREAD > 0) */

	return rs ;
}
/* end subroutine (sigblock_finish) */


