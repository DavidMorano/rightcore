/* tmpx_getrunlevel */

/* get the name of the controlling terminal for the current session */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-08-22, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will try to find a "run-level" record in the UTMPX
        database.

	Synopsis:

	int tmpx_getrunlevel(op)
	TMPX		*op ;

	Arguments:

	op		pointer to TMPX object

	Returns:

	<0	error
	>=0	run-level (including '0' meaning no record found)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<tmpx.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int tmpx_getrunlevel(TMPX *op)
{
	TMPX_ENT	ue ;
	TMPX_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		runlevel = 0 ;

	if (op == NULL) return SR_FAULT ;

/* loop through */

	if ((rs = tmpx_curbegin(op,&cur)) >= 0) {

	    while (rs >= 0) {
		rs1 = tmpx_enum(op,&cur,&ue) ;
		if (rs1 == SR_NOTFOUND) break ;
		rs = rs1 ;
		if (rs < 0) break ;

	        if (ue.ut_type == TMPX_TRUNLEVEL) {
		    runlevel = ue.ut_exit.e_termination & UCHAR_MAX ;

#if	CF_DEBUGS
		{
			const char	*up = ue.ut_user ;
			const int	ul = TMPX_LUSER ;
			const char	*lp = ue.ut_line ;
			const int	ll = TMPX_LLINE ;
		debugprintf("tmpx_getrunlevel: line=%t\n",
	        	lp,strlinelen(lp,ll,40)) ;
		debugprintf("tmpx_getrunlevel: user=%t\n",
	        	up,strlinelen(up,ul,40)) ;
		debugprintf("tmpx_getrunlevel: sid=%d\n", ue.ut_pid) ;
		debugprintf("tmpx_getrunlevel: e_exit=\\x%02x\n",
			ue.ut_exit.e_exit) ;
		debugprintf("tmpx_getrunlevel: e_termination=\\x%02x\n",
			ue.ut_exit.e_termination) ;
		debugprintf("tmpx_getrunlevel: session=%d\n", ue.ut_session) ;
		}
#endif /* CF_DEBUGS */

		    break ;
		} /* end if (got run-level) */

	    } /* end while (looping through entries) */

	    tmpx_curend(op,&cur) ;
	} /* end if (cursor) */

	return (rs >= 0) ? runlevel : rs ;
}
/* end subroutine (tmpx_getrunlevel) */


