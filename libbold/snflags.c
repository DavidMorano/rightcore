/* snflags */

/* make string version of some flags */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Ths object is used in the creation of flags strings.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>

#include	"snflags.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* foward references */


/* local variables */


/* exported subroutines */


int snflags_start(sp,bp,bl)
SNFLAGS		*sp ;
char		*bp ;
int		bl ;
{


	memset(sp,0,sizeof(SNFLAGS)) ;
	sp->bp = bp ;
	sp->bl = bl ;
	return SR_OK ;
}
/* end subroutine (snflags_start) */


int snflags_addstr(sp,s)
SNFLAGS		*sp ;
const char	*s ;
{
	int	rs = SR_OK ;


	if (sp->c++ > 0) {
	    const int	ch_comma = ',' ;
	    rs = storebuf_char(sp->bp,sp->bl,sp->bi,ch_comma) ;
	    sp->bi += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(sp->bp,sp->bl,sp->bi,s,-1) ;
	    sp->bi += rs ;
	}

	return rs ;
}
/* end subroutine (snflags_addstr) */


int snflags_finish(sp)
SNFLAGS		*sp ;
{


	return sp->bi ;
}
/* end subroutine (snflags_finish) */



