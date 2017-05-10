/* mkuibang */


#define	CF_DEBUGS	0		/* compile-time switchable debugging */
#define	CF_FULLNAME	0		/* use full-name */
#define	CF_MAILNAME	1		/* use mail-name */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine create a name that combines the nodename and the
	username into a "bangname."

	Synopsis:

	int mkuibang(nbuf,nlen,uip)
	char		nbuf[] ;
	int		nlen ;
	USERINFO	*uip ;

	Arguments:

	nbuf		buffer to receive resulting name
	nlen		length of supplied buffer
	uip		pointer to USERINFO object

	Returns:

	>=0		OK
	<0		bad


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<userinfo.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARNAME
#define	VARNAME		"NAME"
#endif

#define	NFNAME		"/tmp/mkuibang.nd"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy6(char *,int,cchar *,cchar *,cchar *,cchar *,
			cchar *,cchar *) ;


/* local structures */


/* exported subroutines */


int mkuibang(char rbuf[],int rlen,USERINFO *uip)
{
	int		rs = SR_OK ;
	const char	*np = NULL ;

	if (rbuf == NULL) return SR_FAULT ;
	if (uip == NULL) return SR_FAULT ;

	if (rlen < 0)
	    rlen = NODENAMELEN ;

	rbuf[0] = '\0' ;

#if	CF_FULLNAME
	if (np == NULL) {
	    if ((uip->fullname != NULL) && (uip->fullname[0] != '\0')) {
	        np = uip->fullname ;
	    }
	}
#endif /* CF_FULLNAME */

#ifdef	COMMENT
	{
	    const char	*vnp ;
	    if (np == NULL) {
	        if (((vnp = getenv(VARNAME)) != NULL) && (vnp[0] != '\0')) {
	            np = vnp ;
		}
	    }
	}

	if (np == NULL) {
	    if ((uip->realname != NULL) && (uip->realname[0] != '\0')) {
	        np = uip->realname ;
	    }
	}
#endif /* COMMENT */

	if (np == NULL) {
	    if ((uip->name != NULL) && (uip->name[0] != '\0')) {
	        np = uip->name ;
	    }
	}

#if	CF_MAILNAME
	if (np == NULL) {
	    if ((uip->mailname != NULL) && (uip->mailname[0] != '\0')) {
	        np = uip->mailname ;
	    }
	}
#endif /* CF_MAILNAME */

	if (np == NULL) {
	    if ((uip->fullname != NULL) && (uip->fullname[0] != '\0')) {
	        np = uip->fullname ;
	    }
	}

	{
	    const char	*nn = uip->nodename ;
	    const char	*un = uip->username ;

	    rs = SR_NOTFOUND ;
	    if (np != NULL) {
	       rs = sncpy6(rbuf,rlen,nn,"!",un," (",np,")") ;
	    }

	    if ((rs == SR_OVERFLOW) || (np == NULL)) {
	       rs = sncpy3(rbuf,rlen,nn,"!",un) ;
	    }

	} /* end block */

	return rs ;
}
/* end subroutine (mkuibang) */


