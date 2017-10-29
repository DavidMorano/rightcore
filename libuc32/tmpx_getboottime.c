/* tmpx_getboottime */

/* get the name of the controlling terminal for the current session */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-01-10, David A­D­ Morano
	This subroutine was originally written. 

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine tries to fine the "boot-time" record in the
	UTMPX database.

	Synopsis:

	int tmpx_getboottime(op,rp)
	TMPX		*op ;
	time_t		*rp ;

	Arguments:

	op		pointer to TMPX object
	rp		pointer to resulting time (time_t)

	Returns:

	>=0	database index of the "boot-time" record
	<0	error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<tmpx.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int tmpx_getboottime(TMPX *op,time_t *rp)
{
	TMPX_CUR	uc ;
	TMPX_ENT	ue ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

/* loop through */

	*rp = 0 ;
	if ((rs = tmpx_curbegin(op,&uc)) >= 0) {

	    while (rs >= 0) {
	        rs1 = tmpx_enum(op,&uc,&ue) ;
	        if (rs1 == SR_NOTFOUND) break ;
	        rs = rs1 ;

	        if ((rs >= 0) && (ue.ut_type == TMPX_TBOOTTIME)) {
	            *rp = ue.ut_tv.tv_sec ;
	            break ;
	        }

	        n += 1 ;
	    } /* end while */

	    tmpx_curend(op,&uc) ;
	} /* end if (cursor) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (tmpx_getboottime) */


