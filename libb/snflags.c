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


int snflags_start(SNFLAGS *op,char *bp,int bl)
{
	if (op == NULL) return SR_FAULT ;
	if (bp == NULL) return SR_FAULT ;
	memset(op,0,sizeof(SNFLAGS)) ;
	op->bp = bp ;
	op->bl = bl ;
	*bp = '\0' ;
	return SR_OK ;
}
/* end subroutine (snflags_start) */


int snflags_addstr(SNFLAGS *op,cchar *s)
{
	int		rs = SR_OK ;

	if (op->c++ > 0) {
	    const int	ch_comma = ',' ;
	    rs = storebuf_char(op->bp,op->bl,op->bi,ch_comma) ;
	    op->bi += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(op->bp,op->bl,op->bi,s,-1) ;
	    op->bi += rs ;
	}

	return rs ;
}
/* end subroutine (snflags_addstr) */


int snflags_finish(SNFLAGS *op)
{

	return op->bi ;
}
/* end subroutine (snflags_finish) */


