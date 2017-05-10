/* mkfromname */

/* create a FROM-name */


#define	CF_DEBUGS	0		/* not-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates FROM-name for use in making mail addresses
	(always FROM-type addresses).

	Synopsis:

	int mkfromname(pip)
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
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif


/* external subroutines */

extern int	pcsfullname(const char *,char *,int,const char *) ;
extern int	pcsname(const char *,char *,int,const char *) ;
extern int	isNotPresent(int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkfromname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		nbl = 0 ;

	if (! pip->f.init_fromname) {
	    const int	nlen = REALNAMELEN ;
	    const char	*nbp ;
	    char	nbuf[REALNAMELEN + 1] ;

	    pip->f.init_fromname = TRUE ;

/* for a real-name: first try the PCS facility (we are a PCS program!) */

	    nbp = nbuf ;
	    rs = pcsfullname(pip->pr,nbuf,nlen,pip->username) ;
	    nbl = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("mkfromname: pcsfullname() rs=%d namebuf=>%s<\n",
	            rs,namebuf) ;
#endif

	    if (isNotPresent(rs)) {
	        nbp = nbuf ;
	        rs = pcsname(pip->pr,nbuf,nlen,pip->username) ;
	        nbl = rs ;
	    }

	    if (isNotPresent(rs)) {
	        rs = SR_OK ;
	        nbuf[0] = '\0' ;
	    }

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

	        if ((nbp != NULL) && (nbp[0] != '\0')) {
		    cchar	**vpp = &pip->fromname ;
	            rs = proginfo_setentry(pip,vpp,nbp,nbl) ;
	        }

	    } /* end if (ok) */
	} else {
	    if (pip->fromname != NULL) {
	        nbl = strlen(pip->fromname) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkfromname: ret rs=%d fromname=>%s<\n",
	        rs, pip->fromname) ;
#endif

	return (rs >= 0) ? nbl : rs ;
}
/* end subroutine (mkfromname) */


