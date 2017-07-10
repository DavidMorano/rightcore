/* ourmsginfo */

/* load up the process IDs */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable */


/* revision history:

	= 1994-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine will load up the process user-group IDs.

	Synopsis:

	int ourmsginfo_start(op,dp)
	OURMSGINFO	*op ;
	DATER		*dp ;

	Arguments:

	op		pointer to object
	dp		pointer to DATER object

	Returns:

	<0	error
	>=0	length of found filepath


*****************************************************************************/


#define	OURMSGINFO_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<dater.h>
#include	<localmisc.h>

#include	"ourmsginfo.h"


/* local defines */

#define	OURMSGINFO_N	10


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int ourmsginfo_start(op,dp)
OURMSGINFO	*op ;
DATER		*dp ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(OURMSGINFO)) ;

	rs = dater_startcopy(&op->edate,dp) ;

	return rs ;
}
/* end subroutine (ourmsginfo_start) */


int ourmsginfo_finish(op)
OURMSGINFO	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	for (i = 0 ; i < ourmsginfohead_overlast ; i += 1) {
	    if ((op->hif >> i) & 1) {
	        rs1 = vecstr_finish((op->head + i)) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = dater_finish(&op->edate) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (ourmsginfo_finish) */


int ourmsginfo_addhead(op,w,s,slen)
OURMSGINFO	*op ;
int		w ;
const char	s[] ;
int		slen ;
{
	int		rs = SR_OK ;
	int		opts ;

	if (op == NULL) return SR_FAULT ;
	if (s == NULL) return SR_FAULT ;

	if ((w < 0) || (w >= ourmsginfohead_overlast)) return SR_INVALID ;

	if (slen < 0)
	    slen = strlen(s) ;

	if (slen > 0) {

/* is the vector for this header initialized? */

	    if (! ((op->hif >> w) & 1)) {
	        opts = 0 ;
	        rs = vecstr_start((op->head + w),OURMSGINFO_N,opts) ;
	        op->hif |= (1 << w) ;
	    } /* end if (needed initialization) */
    
	    if (rs >= 0) {
	        rs = vecstr_add((op->head + w),s,slen) ;
	    }

	} /* end if (positive) */

	return rs ;
}
/* end subroutine (ourmsginfo_addhead) */


int ourmsginfo_gethead(op,w,i,rpp)
OURMSGINFO	*op ;
int		w ;
int		i ;
const char	**rpp ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if ((w < 0) || (w >= ourmsginfohead_overlast)) return SR_INVALID ;

	rs = vecstr_get((op->head + w),i,rpp) ;

	return rs ;
}
/* end subroutine (ourmsginfo_gethead) */


