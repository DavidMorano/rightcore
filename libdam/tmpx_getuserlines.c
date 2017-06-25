/* tmpx_getuserlines */

/* get all terminal lines where the given user is logged in on */


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
        terminal-lines for the specified username, if there are any.

	Synopsis:

	int tmpx_getuserlines(op,lp,username)
	TMPX		*op ;
	VECSTR		*lp ;
	const char	username[] ;

	Arguments:

	op		pointer to TMPX object
	lp		pointer to VECSTR to receive terminal-line strings
	username	session ID to find controlling terminal for

	Returns:

	<0		error
	>=0		number of entries returned


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<tmpx.h>
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


int tmpx_getuserlines(TMPX *op,VECSTR *lp,cchar *username)
{
	TMPX_ENT	ue ;
	TMPX_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lp == NULL) return SR_FAULT ;
	if (username == NULL) return SR_FAULT ;
	if (username[0] == '\0') return SR_INVALID ;

/* loop through */

	if ((rs = tmpx_curbegin(op,&cur)) >= 0) {
	    const int	llen = TMPX_LLINE ;
	    int		f ;

	    while (rs >= 0) {
		rs1 = tmpx_fetchuser(op,&cur,&ue,username) ;
		if (rs1 == SR_NOTFOUND) break ;
		rs = rs1 ;
		if (rs < 0) break ;

		f = TRUE ;
	        f = f && (ue.ut_type == TMPX_TUSERPROC) ;
	        f = f && (ue.ut_line[0] != '\0') ;
#ifdef	COMMENT
	        f = f && (strncmp(username,ue.ut_user,TMPX_LUSER) == 0) ;
#endif
		if (f) {
		    n += 1 ;
	            rs = vecstr_add(lp,ue.ut_line,llen) ;
		}

	    } /* end while (looping through entries) */

	    tmpx_curend(op,&cur) ;
	} /* end if (cursor) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (tmpx_getuserlines) */


