/* getlogname */

/* get user information from PASSWD database */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_GETUTMPENT	1		/* use 'getutmpent(3dam)' */
#define	CF_GETUTMPNAME	0		/* use 'getutmpname(3dam)' */
#define	CF_ENV		0		/* use environment variable */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get the login user name.

	Synopsis:

	int getlogname(rbuf,rlen)
	char	rbuf[] ;
	int	rlen ;

	Arguments:

	rbuf		buffer to hold resulting logname
	rlen		length of user supplied buffer

	Returns:

	>=0		length of user logname
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getutmpent.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LOGNAMELEN
#ifdef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARUSER
#define	VARUSER		"USER"
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */


/* exported subroutines */


int getlogname(char *rbuf,int rlen)
{
	int		rs = SR_NOTFOUND ;

	if (rbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if (rlen < 0) rlen = LOGNAMELEN ;

/* check the 'LOGNAME' environment variable */

#if	CF_ENV
	if (rs == SR_NOTFOUND) {
	    cchar	*cp ;

	    if ((cp = getenv(VARLOGNAME)) != NULL) {
	        if (cp[0] != '\0') {
	            rs = sncpy1(rbuf,rlen,cp) ;
		}
	    }

#if	CF_DEBUGS
		debugprintf("getlogname: var rs=%d logname=%s\n",rs,logname) ;
#endif

	} /* end if (LOGNAME environment) */
#endif /* CF_ENV */

/* check the UTMP database */

	if (rs == SR_NOTFOUND) {

#if	CF_GETUTMPENT
	    {
	        GETUTMPENT	e ;
	        if ((rs = getutmpent(&e,0)) >= 0) {
	            rs = sncpy1(rbuf,rlen,e.user) ;
		}
	    }
#else /* CF_GETUTMPENT */

	    if (rs >= 0) {
#if	CF_GETUTMPNAME
	        rs = getutmpname(rbuf,rlen,0) ;
#else /* CF_GETUTMPNAME */
	        rs = uc_getlogin(rbuf,rlen) ;
#endif /* CF_GETUTMPNAME */
	    }

#endif /* CF_GETUTMPENT */

	} /* end if (checking UTMP) */

/* get out */

#if	CF_DEBUGS
	debugprintf("getlogname: ret rs=%d logname=%s\n",rs,logname) ;
#endif

	return rs ;
}
/* end subroutine (getlogname) */


