/* progentry */

/* print entry information */


#define	CF_DEBUGS 	0		/* compile-time debugging */
#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 1994-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine provides some miscellaneous information
	on the recipients in an email message.

	Synopsis:

	int progentryinfo(pip,ep,ind,ofp)
	struct proginfo	*pip ;
	EMA_ENT	*ep ;
	int		ind ;
	bfile		*ofp ;

	Arguments:

	- pip		program information pointer
	- ep		pointer to entry
	- ind	indation level
	- ofp		output file object pointer

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<msg.h>
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

static const char	*blanks = "              " ;


/* exported subroutines */


int progentryinfo(pip,ofp,ep,ind)
struct proginfo	*pip ;
bfile		*ofp ;
EMA_ENT		*ep ;
int		ind ;
{
	int	rs ;
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

	if (ind > MAXINDENT) ind = MAXINDENT ;

	alen = 0 ;
	if (ep->ap != NULL) {
	    any = ep->ap ;
	    alen = ep->al ;
	} else if (ep->rp != NULL) {
	    any = ep->rp ;
	    alen = ep->rl ;
	}

	fmt = (ep->cp != NULL) ? "%s%t %t (%t)\n" : "%s%t %t\n" ;

	rs = bprintf(ofp,fmt,
	    atype(ep->type),blanks,ind,
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


int progentryaddr(pip,ofp,ep,ind)
struct proginfo	*pip ;
bfile		*ofp ;
EMA_ENT		*ep ;
int		ind ;
{
	struct cmd_local	*lsp = pip->lsp ;

	int	rs = SR_OK ;
	int	spc = 0 ;
	int	wlen = 0 ;


#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progentryaddr: ent ind=%u\n",ind) ;
#endif

	if (ep == NULL)
	    return SR_FAULT ;

	if (ind > MAXINDENT) ind = MAXINDENT ;

	if ((rs >= 0) && (ind > 0)) {
	    rs = bwrite(ofp,blanks,ind) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && lsp->af.original) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progentryaddr: ORIG\n") ;
#endif

	    if (ep->op != NULL) {
	        rs = bprintf(ofp,"»%s«",ep->op) ;
	        wlen += rs ;
	    }

	    spc += 1 ;
	}

	if ((rs >= 0) && lsp->af.best) {
	    const char	*vp ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progentryaddr: BEST\n") ;
#endif

	    if (spc > 0) {
	        rs = bwrite(ofp," ",2) ;
	        wlen += rs ;
	    }

	    if (ep->rp != NULL) {
	        vp = ep->rp ;
	    } else if (ep->ap != NULL) {
	        vp = ep->ap ;
	    } else if (ep->cp != NULL)
	        vp = ep->cp ;

	    if (rs >= 0) {
	        rs = bwrite(ofp,vp,-1) ;
	        wlen += rs ;
	    }

	    spc += 1 ;
	}

	if ((rs >= 0) && lsp->af.any) {
	    const char	*vp ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progentryaddr: ANY\n") ;
#endif

	    if (ep->ap != NULL) { /* address */
	        vp = ep->ap ;
	    } else if (ep->rp != NULL) { /* route */
	        vp = ep->rp ;
	    } else if (ep->cp != NULL) /* comment */
	        vp = ep->cp ;

	    rs = bprintf(ofp,"ª%sª",vp) ;
	    wlen += rs ;
	    spc += 1 ;
	}

	if ((rs >= 0) && lsp->af.address) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("progentryaddr: ADDR\n") ;
	        debugprintf("progentryaddr: al=%u a=>%s<\n",
	            strlen(ep->ap),ep->ap) ;
	    }
#endif

	    if (spc > 0) {
	        rs = bwrite(ofp," ",1) ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && (ep->ap != NULL)) {
	        rs = bwrite(ofp,ep->ap,-1) ;
	        wlen += rs ;
	    }

	    spc += 1 ;
	}

	if ((rs >= 0) && lsp->af.route) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progentryaddr: ROUTE\n") ;
#endif

	    if (spc > 0) {
	        rs = bputc(ofp,' ') ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && (ep->rp != NULL)) {
	        rs = bwrite(ofp,ep->rp,-1) ;
	        wlen += rs ;
	    }

	    spc += 1 ;
	}

	if ((rs >= 0) && lsp->af.comment) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progentryaddr: COMM\n") ;
#endif

	    if (spc > 0) {
	        rs = bputc(ofp,' ') ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && (ep->cp != NULL)) {
	        rs = bprintf(ofp,"(%s)",ep->cp) ;
	        wlen += rs ;
	    }

	    spc += 1 ;
	}

	if (rs >= 0) {
	    rs = bprintf(ofp,"\n") ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progentryaddr: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

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


