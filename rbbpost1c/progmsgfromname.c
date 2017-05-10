/* progmsgfromname */

/* create a mail-msg FROM-name */


#define	CF_DEBUGS	0		/* not-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine creates FROM-name for use in making mail addresses
	(always FROM-type addresses).

	Synopsis:

	int progmsgfromname(pip)
	struct proginfo	*pip ;

	Arguments:

	pip		pointer to program information

	Returns:

	>=0		OK
	<0		error


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
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


/* external subroutines */

extern int	pcsfullname(const char *,char *,int,const char *) ;
extern int	pcsname(const char *,char *,int,const char *) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progmsgfromname(pip)
struct proginfo	*pip ;
{
	int	rs = SR_OK ;
	int	nbl = 0 ;


	if (pip->msgfromname == NULL) {
	    char	namebuf[REALNAMELEN + 1] ;
	    const char	*nbp ;

/* for a real-name: first try the PCS facility (we are a PCS program!) */

	    nbp = namebuf ;
	    rs = pcsfullname(pip->pr,namebuf,REALNAMELEN,pip->username) ;
	    nbl = rs ;
	    if ((rs == SR_NOTFOUND) || (rs == SR_ACCESS)) {
	        rs = pcsname(pip->pr,namebuf,REALNAMELEN,pip->username) ;
	        nbl = rs ;
	    }
	    if ((rs == SR_NOTFOUND) || (rs == SR_ACCESS))
	        namebuf[0] = '\0' ;

	    if (rs >= 0) {

/* try USERINFO-derived possibilities */

	        if ((nbp == NULL) || (nbp[0] == '\0')) {
	            nbp = pip->fullname ;
	            nbl = -1 ;
	        }

	        if ((nbp == NULL) || (nbp[0] == '\0')) {
	            nbp = pip->name ;
	            nbl = -1 ;
	        }

	        if ((nbp == NULL) || (nbp[0] == '\0')) {
	            rs = proginfo_setentry(pip,&pip->msgfromname,nbp,nbl) ;
	        }

	    } /* end if */
	} else {
	    nbl = strlen(pip->msgfromname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkfromname: ret rs=%d fromname=>%s<\n",
	        rs, pip->msgfromname) ;
#endif

	return (rs >= 0) ? nbl : rs ;
}
/* end subroutine (progmsgfromname) */



