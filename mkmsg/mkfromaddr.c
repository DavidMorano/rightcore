/* mkfromaddr */

/* create a mail-msg FROM-address */


#define	CF_DEBUGS	0		/* not-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a default FROM-address.

	Synopsis:

	int mkfromaddr(pip)
	PROGINFO	*pip ;

	Arguments:

	pip		pointer to program information

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<buffer.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	MABUFLEN
#define	MABUFLEN	((3 * MAXHOSTNAMELEN) + 1024)
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	VARMAILFROM
#define	VARMAILFROM	"MAILFROM"
#endif


/* external subroutines */

extern int	mkfromname(PROGINFO *) ;


/* local structures */


/* forward references */

static int	procfrom(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int mkfromaddr(PROGINFO *pip)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkfromaddr: ent from=>%s<\n",pip->hdr_from) ;
#endif

	if (pip->hdr_from == NULL) {
	    cchar	*cp = NULL ;
	    if ((cp = getenv(VARMAILFROM)) != NULL) {
		if (cp[0] != '\0') {
		    pip->hdr_from = cp ;
		    rs = strlen(cp) ;
		}
	    }
	    if (pip->hdr_from == NULL) {
		rs = procfrom(pip) ;
	    }
	} else {
	    if (pip->hdr_from != NULL) {
	        rs = strlen(pip->hdr_from) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("mkfromaddr: ret rs=%d\n",rs) ;
	    debugprintf("mkfromaddr: ret from=>%s<\n",pip->hdr_from) ;
	}
#endif

	return rs ;
}
/* end subroutine (mkfromaddr) */


/* local subroutines */


static int procfrom(PROGINFO *pip)
{
	BUFFER		b ;
	int		rs ;
	int		rs1 ;
	int		bl = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkfromaddr/procfrom: ent\n") ;
#endif

	if ((rs = buffer_start(&b,MABUFLEN)) >= 0) {

	    buffer_strw(&b,pip->username,-1) ;
	    buffer_char(&b,'@') ;
	    buffer_strw(&b,pip->domainname,-1) ;

/* add a name if we can find one */

	    if (! pip->f.init_fromname) {
	        rs = mkfromname(pip) ;
	    }

	    if ((rs >= 0) && (pip->fromname != NULL)) {
	        buffer_char(&b,' ') ;
	        buffer_char(&b,CH_LPAREN) ;
	        buffer_strw(&b,pip->fromname,-1) ;
	        buffer_char(&b,CH_RPAREN) ;
	    } /* end if (adding name) */

	    if (rs >= 0) {
	        cchar	*bp ;
	        if ((rs = buffer_get(&b,&bp)) >= 0) {
		    cchar	**vpp = &pip->hdr_from ;
	            bl = rs ;
	            rs = proginfo_setentry(pip,vpp,bp,bl) ;
		}
	    }

	    rs1 = buffer_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkfromaddr/procfrom: ret rs=%d bl=%u\n",rs,bl) ;
#endif

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (procfrom) */


