/* mkreplyaddr */

/* create a mail REPLYTO-address */


#define	CF_DEBUGS	0		/* not-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a mail REPLYTO-address (if we can).

	Synopsis:

	int mkreplyaddr(pip)
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

#ifndef	VARMAILREPLY
#define	VARMAILREPLY	"MAILREPLY"
#endif


/* external subroutines */

extern int	mkfromname(PROGINFO *) ;


/* local structures */


/* forward references */

static int	loadreply(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int mkreplyaddr(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (! pip->f.init_replyto) {
	    pip->f.init_replyto = TRUE ;
	    if (pip->hdr_replyto == NULL) {
	        if (pip->hdr_replyto == NULL) {
		    cchar	*cp = NULL ;
	            if ((cp = getenv(VARMAILREPLY)) != NULL) {
	                if (cp[0] != '\0') {
	                    pip->hdr_replyto = cp ;
			    rl = strlen(cp) ;
	                }
	            }
	        }
	        if (pip->hdr_replyto == NULL) {
	            rs = loadreply(pip) ;
	 	    rl = rs ;
	        }
	    } /* end if (more needed) */
	} else {
	    if (pip->hdr_replyto != NULL) {
		rl = strlen(pip->hdr_replyto) ;
	    }
	} /* end if (needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkreplyaddr: ret rs=%d replyto=>%s<\n",
	        rs, pip->hdr_replyto) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (mkreplyaddr) */


/* local subroutines */


static int loadreply(PROGINFO *pip)
{
	BUFFER		b ;
	int		rs ;
	int		rs1 ;
	int		bl = 0 ;
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
	            cchar	**vpp = &pip->hdr_replyto ;
	            bl = rs ;
	            rs = proginfo_setentry(pip,vpp,bp,bl) ;
	        }
	    } /* end if (ok) */

	    rs1 = buffer_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */
	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (loadreply) */


