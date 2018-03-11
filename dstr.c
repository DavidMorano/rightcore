/* dstr */

/* string object */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object tries to mimic a dynamic-length string.


*******************************************************************************/


#define	DSTR_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"dstr.h"


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


int dstr_start(DSTR *sop,cchar *s,int slen)
{
	int		rs = SR_NOEXIST ;

	sop->sbuf = NULL ;
	sop->slen = 0 ;
	if (s != NULL) {
	    sop->slen = (slen < 0) ? strlen(s) : slen ;
	    if ((rs = uc_malloc(sop->slen,&sop->sbuf)) >= 0) {
	        rs = sop->slen ;
	        strwcpy(sop->sbuf,s,sop->slen) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (dstr_start) */


int dstr_finish(DSTR *sop)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sop->sbuf != NULL) {
	    rs1 = uc_free(sop->sbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    sop->sbuf = NULL ;
	}

	sop->slen = 0 ;
	return rs ;
}
/* end subroutine (dstr_finish) */


int dstr_assign(DSTR *sop,DSTR *sop2)
{
	int		rs ;

	if ((rs = dstr_finish(sop)) >= 0) {
	    rs = dstr_start(sop,sop2->sbuf,sop2->slen) ;
	}

	return rs ;
}
/* end subroutine (dstr_assign) */


