/* prognote */

/* Program-Note (TERMNOTE) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine was borrowed and modified from previous generic
	front-end 'main' subroutines!

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we do some TERMNOTE handling.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"defs.h"


/* local defines */


/* external subroutines */

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int prognote_begin(PROGINFO *pip)
{
	int		rs ;
	if ((rs = termnote_open(&pip->tn,pip->pr)) >= 0) {
	    rs = ptm_create(&pip->tmutex,NULL) ;
	    if (rs < 0) {
		termnote_close(&pip->tn) ;
	    }
	}
	return rs ;
}
/* end subroutine (prognote_begin) */


int prognote_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = ptm_destroy(&pip->tmutex) ;
	if (rs >= 0) rs = rs1 ;
	rs1 = termnote_close(&pip->tn) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (prognote_end) */


int prognote_write(pip,recips,max,o,np,nl)
PROGINFO	*pip ;
const char	**recips ;
int		max ;
int		o ;
const char	np[] ;
int		nl ;
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if ((rs = ptm_lock(&pip->tmutex)) >= 0) {
	    rs = termnote_write(&pip->tn,recips,max,o,np,nl) ;
	    len = rs ;
	    rs1 = ptm_unlock(&pip->tmutex) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex-lock) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (prognote_write) */


int prognote_check(PROGINFO *pip)
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if ((rs = ptm_lock(&pip->tmutex)) >= 0) {
	    rs = termnote_check(&pip->tn,pip->daytime) ;
	    len = rs ;
	    rs1 = ptm_unlock(&pip->tmutex) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm-lock) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (prognote_check) */


