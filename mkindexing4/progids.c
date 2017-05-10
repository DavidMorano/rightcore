/* progids */

/* manage process IDs */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
        This subroutine was borrowed and modified from some previous module.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage the general process logging.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matpstr(cchar **,int,cchar *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progids_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progids_begin: ent\n") ;
#endif

	if (! pip->open.ids) {
	    if ((rs = ids_load(&pip->id)) >= 0) {
		pip->open.ids = TRUE ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progids_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progids_begin) */


int progids_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.ids) {
	    pip->open.ids = FALSE ;
	    rs1 = ids_release(&pip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (progids_end) */


extern int progids_sperm(PROGINFO *pip,struct ustat *sbp,int am)
{
	int		rs ;
	if (pip->open.ids) {
	   rs = sperm(&pip->id,sbp,am) ;
	} else {
	    rs = SR_NOTOPEN ;
	}
	return rs ;
}
/* end subroutine (progids_sperm) */


