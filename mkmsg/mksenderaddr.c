/* mksenderaddr */

/* create a mail SENDER-address */


#define	CF_DEBUGS	0		/* not-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_SENDERNAME	1		/* add a sender name */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a mail SENDER-address (if we can).

	Synopsis:

	int mksenderaddr(pip)
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

#ifndef	VARMAILSENDER
#define	VARMAILSENDER	"MAILSENDER"
#endif


/* external subroutines */

extern int	mkfromname(PROGINFO *) ;


/* local structures */


/* forward references */

static int	loadsender(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int mksenderaddr(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (! pip->f.init_sender) {
	    pip->f.init_sender = TRUE ;
	    if (pip->hdr_sender == NULL) {
	        cchar	*cp = NULL ;
	        if ((cp = getenv(VARMAILSENDER)) != NULL) {
	            if (cp[0] != '\0') {
	                pip->hdr_sender = cp ;
			rl = strlen(pip->hdr_sender) ;
	            }
	        }
	        if (pip->hdr_sender == NULL) {
	            rs = loadsender(pip) ;
		    rl = rs ;
	        }
	    }
	} else {
	    if (pip->hdr_sender != NULL) {
		rl = strlen(pip->hdr_sender) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mksenderaddr: ret rs=%d sender=>%s<\n",
	        rs,pip->hdr_sender) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (mksenderaddr) */


/* local subroutines */


static int loadsender(PROGINFO *pip)
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

#if	CF_SENDERNAME
	    if ((rs = mkfromname(pip)) >= 0) {
	        if (pip->fromname != NULL) {
	            buffer_char(&b,' ') ;
	            buffer_char(&b,CH_LPAREN) ;
	            buffer_strw(&b,pip->fromname,-1) ;
	            buffer_char(&b,CH_RPAREN) ;
	        } /* end if (adding name) */
	    } /* end if 9mkfromname) */
#endif /* CF_SENDERNAME */

	    if (rs >= 0) {
	        cchar	*bp ;
	        if ((rs = buffer_get(&b,&bp)) >= 0) {
	            const char	**vpp = &pip->hdr_sender ;
	            bl = rs ;
	            rs = proginfo_setentry(pip,vpp,bp,bl) ;
	        }
	    }

	    rs1 = buffer_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */
	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (loadsender) */


