/* ucontext_rtn */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Retrives the return-address from the UCONTEXT structure.

	Q. Why the name of this subroutine?
        A. The method name 'rtn' was already used for a member of a register
        window structure on the SPARC platform, so we adopted it.


*******************************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<ucontext.h>

#include	<vsystem.h>
#include	<localmisc.h>
#include	<exitcodes.h>


/* local defines */


/* external variables */

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern const char	*strsigabbr(int) ;


/* local structures */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


/* forward references */


/* local variables */


/* exported subroutines */


int ucontext_rtn(ucontext_t *ucp,long *rpp)
{
	mcontext_t	*mcp = &ucp->uc_mcontext ;
	greg_t		*rp ;
	rp = mcp->gregs ;
	if (rpp != NULL) *rpp = rp[(_NGREG-1)] ;
	return SR_OK ;
}
/* end subroutine (ucontext_rtn) */


