/* utmpaccent */

/* UTMPACCENT management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines perform some management function on an UTMPACCENT
        object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<localmisc.h>

#include	"utmpaccent.h"


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	si_copystr(STOREITEM *,const char **,const char *,int) ;


/* local variables */


/* exported subroutines */


int utmpaccent_load(UTMPACCENT *uep,char *uebuf,int uelen,const UTMPFENT *suep)
{
	STOREITEM	ib ;
	int		rs ;
	int		rs1 ;

	if ((rs = storeitem_start(&ib,uebuf,uelen)) >= 0) {

	    uep->ctime = suep->ut_tv.tv_sec ;
	    uep->sid = suep->ut_pid ;
	    uep->session = suep->ut_session ;
	    uep->type = suep->ut_type ;
	    uep->syslen = suep->ut_syslen ;
	    uep->e_exit = suep->ut_exit.e_exit ;
	    uep->e_term = suep->ut_exit.e_termination ;
	    strwcpy(uep->id,suep->ut_id,4) ;

	    si_copystr(&ib,&uep->user,suep->ut_user,UTMPACCENT_LUSER) ;

	    si_copystr(&ib,&uep->line,suep->ut_line,UTMPACCENT_LLINE) ;

	    si_copystr(&ib,&uep->host,suep->ut_host,UTMPACCENT_LHOST) ;

	    rs1 = storeitem_finish(&ib) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (utmpaccent_load) */


int utmpaccent_size(const UTMPACCENT *uep)
{
	int		size = 0 ;
	if (uep->user != NULL) {
	    size += (strlen(uep->user)+1) ;
	}
	if (uep->line != NULL) {
	    size += (strlen(uep->line)+1) ;
	}
	if (uep->host != NULL) {
	    size += (strlen(uep->host)+1) ;
	}
	return size ;
}
/* end subroutine (utmpaccent_size) */


/* local subroutines */


static int si_copystr(STOREITEM *ibp,cchar **pp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	const char	**cpp = (const char **) pp ;

	*cpp = NULL ;
	if (sp != NULL) {
	    rs = storeitem_strw(ibp,sp,sl,cpp) ;
	}

	return rs ;
}
/* end subroutine (si_copystr) */


