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
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getax.h>
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


int getlogname(logname,lognamelen)
char		logname[] ;
int		lognamelen ;
{
	int		rs = SR_NOTFOUND ;

	if (logname == NULL) return SR_FAULT ;

	logname[0] = '\0' ;
	if (lognamelen < 0)
	    lognamelen = LOGNAMELEN ;

/* check the 'LOGNAME' environment variable */

#if	CF_ENV
	if (rs == SR_NOTFOUND) {
	    const char	*cp ;

	    if ((cp = getenv(VARLOGNAME)) != NULL) {

	        if (cp[0] != '\0')
	            rs = sncpy1(logname,lognamelen,cp) ;

	    }

#if	CF_DEBUGS
		debugprintf("getlogname: var rs=%d logname=%s\n",
			rs,logname) ;
#endif

	} /* end if (LOGNAME environment) */
#endif /* CF_ENV */

/* check the UTMP database */

	if (rs == SR_NOTFOUND) {

#if	CF_GETUTMPENT
	    {
	        GETUTMPENT	e ;


	        rs = getutmpent(&e,0) ;

	        if (rs >= 0)
	            rs = sncpy1(logname,lognamelen,e.user) ;

#if	CF_DEBUGS
		debugprintf("getlogname: getutmpent rs=%d logname=%s\n",
			rs,logname) ;
#endif

	    }
#else /* CF_GETUTMPENT */

#if	CF_GETUTMPNAME
	    rs = getutmpname(logname,lognamelen,0) ;

#if	CF_DEBUGS
		debugprintf("getlogname: getutmpname rs=%d logname=%s\n",
			rs,logname) ;
#endif

#else /* CF_GETUTMPNAME */

	    rs = uc_getlogin(logname,lognamelen) ;

#if	CF_DEBUGS
		debugprintf("getlogname: getlogin rs=%d logname=%s\n",
			rs,logname) ;
#endif

#endif /* CF_GETUTMPNAME */

#endif /* CF_GETUTMPENT */

	} /* end if (checking UTMP) */

/* get out */

#if	CF_DEBUGS
	debugprintf("getlogname: ret rs=%d logname=%s\n",
		rs,logname) ;
#endif

	return rs ;
}
/* end subroutine (getlogname) */


