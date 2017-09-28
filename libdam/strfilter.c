/* strfilter */

/* filter a string of text against some criteria */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module filters a line of text against one or both of a
        "select" list and an "exclude" list.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"strfilter.h"


/* local defines */


/* external subroutines */

extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	sisub(const char *,int,const char *) ;
extern int	vecstr_loadfile(vecstr *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int strfilter_start(STRFILTER *op,cchar *ssfname,cchar *sxfname)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(STRFILTER)) ;

	if ((ssfname != NULL) && (ssfname[0] != '\0')) {
	    op->f.sslist = TRUE ;
	    rs = vecstr_start(&op->sslist,10,0) ;
	    if (rs >= 0) {
	        rs = vecstr_loadfile(&op->sslist,TRUE,ssfname) ;
	        if (rs < 0)
	            vecstr_finish(&op->sslist) ;
	    }
	}

	if (rs >= 0) {
	    if ((sxfname != NULL) && (sxfname[0] != '\0')) {
	        op->f.sxlist = TRUE ;
	        rs = vecstr_start(&op->sxlist,10,0) ;
	        if (rs >= 0) {
	            rs = vecstr_loadfile(&op->sxlist,TRUE,sxfname) ;
	            if (rs < 0)
	                vecstr_finish(&op->sxlist) ;
	        }
	    }
	    if (rs < 0)
	        vecstr_finish(&op->sslist) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (strfilter_start) */


int strfilter_finish(STRFILTER *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->f.sxlist) {
	    op->f.sxlist = FALSE ;
	    rs1 = vecstr_finish(&op->sxlist) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->f.sslist) {
	    op->f.sslist = FALSE ;
	    rs1 = vecstr_finish(&op->sslist) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (strfilter_finish) */


int strfilter_check(STRFILTER *op,cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		i ;
	int		si ;
	int		f = TRUE ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

	if (op->f.sslist) {
	    f = FALSE ;
	    for (i = 0 ; vecstr_get(&op->sslist,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	        if ((si = sisub(lbuf,llen,cp)) >= 0) {
	            f = TRUE ;
	            break ;
	        }
		}
	    }
	} /* end if */

	if (f && op->f.sxlist) {
	    for (i = 0 ; vecstr_get(&op->sxlist,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	        if ((si = sisub(lbuf,llen,cp)) >= 0) {
	            f = FALSE ;
	            break ;
	        }
		}
	    }
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (strfilter_check) */


