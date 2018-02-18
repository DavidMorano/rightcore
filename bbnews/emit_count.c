/* emit_count */

/* counts number of articles on the given newsgroup */


#define	CF_DEBUGS	0		/* compile-time debuging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1994-11-01, David A­D­ Morano
	I wrote this from scratch to support the familiar function of
	getting the count of articles not yet read (whatever)!

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int emit_header(pip,dsp,ai,ap,ngdir,af)
	PROGINFO	*pip ;
	MKDIRLIST_ENT	*dsp ;
	int		ai ;
	ARTLIST_ENT	*ap ;
	char		ngdir[] ;
	char		af[] ;

	Arguments:

	pip		program information pointer
	dsp		user structure pointer
	int		article index within newsgroup
	ap		article ARTLIST_ENT pointer
	ngdir		directory (relative to spool directory) of article
	af		article base file name

	Returns:

	<0		error
	>=0		emit-codes


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>

#include	"artlist.h"
#include	"mkdirlist.h"
#include	"config.h"
#include	"defs.h"


/* external subroutines */


/* external variables */


/* exported subroutines */


int emit_count(pip,dsp,ai,ap,ngdir,af)
PROGINFO	*pip ;
MKDIRLIST_ENT	*dsp ;
int		ai ;
ARTLIST_ENT	*ap ;
const char	ngdir[] ;
const char	af[] ;
{
	int		rs = EMIT_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("emit_count: ent ng=%s af=%s\n",ngdir,af) ;
#endif

	if (ngdir == NULL) return SR_FAULT ;
	if (af == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("emit_count: ret rs=%d\n",rs) ;
#endif

	return rs ;

} /* end subroutine (emit_count) */


