/* tmpx_getsessions */

/* get all of the terminals where the given user is logged in */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
        This subroutine was originally written. It was prompted by the failure
        of other terminal message programs from finding the proper controlling
        terminal.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will find and return the names of all of the controlling
        terminals for the specified username, if there are any.

	Synopsis:

	int tmpx_getsessions(op,vip,username)
	TMPX		*op ;
	VECINT		*vip ;
	const char	username[] ;

	Arguments:

	op		pointer to TMPX object
	vip		pointer to VECINT to receive sessions
	username	username to find sessions

	Returns:

	<0		error
	>=0		number of entries returned


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<tmpx.h>
#include	<vecint.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int tmpx_getsessions(TMPX *txp,VECINT *vip,const char *un)
{
	TMPX_ENT	ue ;
	TMPX_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if (txp == NULL) return SR_FAULT ;
	if (vip == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (un[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("tmpx_getsessions: u=%s\n",un) ;
#endif

	if ((rs = tmpx_curbegin(txp,&cur)) >= 0) {
	    int	f ;
	    while (rs >= 0) {
	        rs1 = tmpx_fetchuser(txp,&cur,&ue,un) ;
	        if (rs1 == SR_NOTFOUND) break ;
	        rs = rs1 ;
	        if (rs < 0) break ;

	        f = TRUE ;
	        f = f && (ue.ut_type == TMPX_TUSERPROC) ;
	        f = f && (ue.ut_line[0] != '\0') ;
	        if (f) {
	            int	v = ue.ut_pid ;
	            n += 1 ;
	            rs = vecint_add(vip,v) ;
	        } /* end if (got one) */

	    } /* end while (looping through entries) */
	    tmpx_curend(txp,&cur) ;
	} /* end if (cursor) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (tmpx_getsessions) */


