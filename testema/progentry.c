/* progentry */

/* print entry information */


#define	CF_DEBUGS 	0		/* compile-time debugging */
#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 1994-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	This subroutine provides some miscellaneous information
	on the recipients in an email message.

	Synopsis:

	int progentryinfo(pip,ep,indent,ofp)
	struct proginfo	*pip ;
	EMA_ENT	*ep ;
	int		indent ;
	bfile		*ofp ;

	Arguments:

	- pip		program information pointer
	- ep		pointer to entry
	- indent	indentation level
	- ofp		output file object pointer

	Returns:

	>=0		OK
	<0		error code


***********************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ema.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"ema_local.h"


/* local defines */

#define	MAXINDENT	7


/* external subroutines */

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* local structures */


/* forward references */

static const char	*atype(int) ;


/* local variables */

static const char	*atypes[] = {
	"R",			/* regular */
	"P",			/* PCS list */
	"L",			/* local alias */
	"S",			/* system-wide alias */
	"G",			/* ARPA group list */
	NULL
} ;

static const char	*blanks[MAXINDENT + 2] = {
	"",
	"  ",
	"    ",
	"      ",
	"        ",
	"          ",
	"            ",
	"              ",
	NULL
} ;


/* exported subroutines */


int progentryinfo(pip,ofp,ep,indent)
struct proginfo	*pip ;
bfile		*ofp ;
EMA_ENT	*ep ;
int		indent ;
{
	int	rs = SR_OK ;
	int	alen ;
	int	wlen = 0 ;

	const char	*fmt ;
	const char	*any = NULL ;


#if	CF_DEBUGS
	    debugprintf("progentryinfo: entered ep(%p)\n",ep) ;
#endif

	if (ep == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progentryinfo: ofp(%p)\n",ofp) ;
#endif

	if (indent > MAXINDENT)
	    indent = MAXINDENT ;

	alen = 0 ;
	if (ep->ap != NULL) {

	    any = ep->ap ;
	    alen = ep->al ;

	} else if (ep->rp != NULL) {

	    any = ep->rp ;
	    alen = ep->rl ;

	}

	fmt = (ep->cp != NULL) ? "%s%s %t (%t)\n" : "%s%s %t\n" ;

	rs = bprintf(ofp,fmt,
	    atype(ep->type),blanks[indent],
	    ((any != NULL) ? any : ""),alen,
	    ((ep->cp != NULL) ? ep->cp : ""),ep->cl) ;

	wlen += rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progentryinfo: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progentryinfo) */


int progentryaddr(pip,ofp,ep,indent)
struct proginfo	*pip ;
bfile		*ofp ;
EMA_ENT	*ep ;
int		indent ;
{
	struct cmd_local	*lsp = pip->lsp ;

	int	rs = SR_OK ;
	int	spc = 0 ;
	int	wlen = 0 ;

	const char	*any = NULL ;


	if (ep == NULL)
	    return SR_FAULT ;

	if (indent > MAXINDENT)
	    indent = MAXINDENT ;

	if ((rs >= 0) && (indent > 0)) {
		rs = bwrite(ofp,blanks,indent) ;
		wlen += rs ;
	}

	if ((rs >= 0) && lsp->af.original) {

	    if (ep->op != NULL) {
	        rs = bprintf(ofp,"%s",ep->op) ;
	        wlen += rs ;
	    }

	    spc += 1 ;
	}

	if ((rs >= 0) && lsp->af.best) {

	    if (ep->rp != NULL) {
	        rs = bprintf(ofp,"%s",ep->rp) ;

	    } else if (ep->ap != NULL) {
	        rs = bprintf(ofp,"%s",ep->ap) ;

	    } else if (ep->cp != NULL)
	        rs = bprintf(ofp,"(%s)",ep->cp) ;

	    wlen += rs ;
	    spc += 1 ;
	}

	if ((rs >= 0) && lsp->af.any) {

	    if (ep->ap != NULL) {
	        rs = bprintf(ofp,"%s",ep->ap) ;

	    } else if (ep->rp != NULL) {
	        rs = bprintf(ofp,"%s",ep->rp) ;

	    } else if (ep->cp != NULL)
	        rs = bprintf(ofp,"(%s)",ep->cp) ;

	    wlen += rs ;
	    spc += 1 ;
	}

	if ((rs >= 0) && lsp->af.address) {

	    if (spc > 0) {
	        rs = bprintf(ofp,"|") ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && (ep->ap != NULL)) {
	        rs = bprintf(ofp,"%s",ep->ap) ;
	        wlen += rs ;
	    }

	    spc += 1 ;
	}

	if ((rs >= 0) && lsp->af.route) {

	    if (spc > 0) {
	        rs = bprintf(ofp,"|") ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && (ep->rp != NULL)) {
	        rs = bprintf(ofp,"%s",ep->rp) ;
	        wlen += rs ;
	    }

	    spc += 1 ;
	}

	if (lsp->af.comment) {

	    rs = 0 ;
	    if (spc > 0) {
	        rs = bprintf(ofp,"|") ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && (ep->cp != NULL)) {
	        rs = bprintf(ofp,"%s",ep->cp) ;
	        wlen += rs ;
	    }

	    spc += 1 ;
	}

	if (rs >= 0) {
	    rs = bprintf(ofp,"\n") ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progentryaddr) */


/* local subroutines */


static const char *atype(type)
int		type ;
{


	return (type < nelements(atypes)) ? atypes[type] : "U" ;
}
/* end subroutine (atype) */



