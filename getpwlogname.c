/* getpwlogname */

/* get the user's PASSWD entry based on logname (not UID) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_GETUTMPNAME	1		/* use 'getutmpname()' */
#define	CF_USERNAME	0		/* use USERNAME environment */
#define	CF_UID		0		/* use UID */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get the PASSWD database structure for the logged in user.

	Synopsis:

	int getpwlogname(pwp,pwbuf,pwbuflen)
	struct passwd	*pwp ;
	char		pwbuf[] ;
	int		pwbuflen ;

	Arguments:

	pwp		'passwd' entry pointer (struct passwd *)
	buf		buffer to hold resulting logname
	buflen		length of user supplied buffer

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARUSER
#define	VARUSER		"USER"
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif

#ifndef	USERNAMELEN
#ifdef	LOGNAMELEN
#define	USERNAMELEN	LOGNAMELEN
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	SR_OK
#define	SR_OK		0
#endif

#ifndef	SR_FAULT
#define	SR_FAULT	(- EFAULT)
#endif

#ifndef	SR_INVALID
#define	SR_INVALID	(- EINVAL)
#endif

#ifndef	SR_NOENT
#define	SR_NOENT	(- ENOENT)
#endif


/* external subroutines */

#if	CF_GETUTMPNAME
extern int	getutmpname(char *,int,pid_t) ;
#endif


/* external variables */


/* forward references */


/* exported subroutines */


int getpwlogname(pwp,pwbuf,pwbuflen)
struct passwd	*pwp ;
char		pwbuf[] ;
int		pwbuflen ;
{
	uid_t	uid = getuid() ;

	int	rs = SR_NOTFOUND ;
	int	pwlen = 0 ;

	const char	*np ;
	const char	*lognamep ;

	char	namebuf[LOGNAMELEN + 1] ;


	if ((pwp == NULL) || (pwbuf == NULL))
	    return SR_FAULT ;

	if (pwbuflen <= 0)
	    return SR_INVALID ;

/* check the 'LOGNAME' environment variable */

	lognamep = NULL ;
	if (rs == SR_NOTFOUND) {

	    lognamep = getenv(VARLOGNAME) ;

	    np = lognamep ;
	    if ((np != NULL) && (np[0] != '\0')) {

	        rs = GETPW_NAME(pwp,pwbuf,pwbuflen,np) ;
		pwlen = rs ;
	        if ((rs >= 0) && (pwp->pw_uid != uid))
	            rs = SR_NOTFOUND ;

	    } /* end if */

	} /* end if (LOGNAME environment) */

/* check the 'UTMP' database */

	if (rs == SR_NOTFOUND) {

#if	CF_GETUTMPNAME
	    rs = getutmpname(namebuf,LOGNAMELEN,0) ;
#else /* CF_GETUTMPNAME */
	    rs = uc_getlogin(namebuf,LOGNAMELEN) ;
#endif /* CF_GETUTMPNAME */

	    if (rs >= 0) {

	        rs = GETPW_NAME(pwp,pwbuf,pwbuflen,namebuf) ;
		pwlen = rs ;
	        if ((rs >= 0) && (pwp->pw_uid != uid))
	            rs = SR_NOTFOUND ;

	    } /* end if */

	} /* end if (trying UTMP) */

#if	CF_USERNAME

/* check the 'USERNAME' environment variable */

	usernamep = NULL ;
	if (rs == SR_NOTFOUND) {

	    usernamep = getenv(VARUSERNAME) ;

	    np = usernamep ;
	    if ((np != NULL) && (np[0] != '\0') &&
		((lognamep != NULL) || (strcmp(lognamep,np) != 0))) {

	        rs = GETPW_NAME(pwp,pwbuf,pwbuflen,np) ;
		pwlen = rs ;
	        if ((rs >= 0) && (pwp->pw_uid != uid))
	            rs = SR_NOTFOUND ;

	    } /* end if */

	} /* end if (USERNAME environment) */

/* check the 'USER' environment variable */

	userp = NULL ;
	if (rs == SR_NOTFOUND) {

	    userp = getenv(VARUSER) ;

	    np = userp ;
	    if ((np != NULL) && (np[0] != '\0') &&
		((lognamep != NULL) || (strcmp(lognamep,np) != 0)) &&
		((usernamep != NULL) || (strcmp(usernamep,np) != 0))) {

	        rs = GETPW_NAME(pwp,pwbuf,pwbuflen,np) ;
		pwlen = rs ;
	        if ((rs >= 0) && (pwp->pw_uid != uid))
	            rs = SR_NOTFOUND ;

	    } /* end if */

	} /* end if (USER environment) */

#endif /* CF_USERNAME */

#if	CF_UID

/* use the UID */

	if (rs == SR_NOTFOUND) {

	    rs = GETPW_UID(pwp,pwbuf,pwbuflen,uid) ;
	    pwlen = rs ;
	    if ((rs >= 0) && (pwp->pw_uid != uid))
	        rs = SR_NOTFOUND ;

	} /* end if (PASSWD attempt) */

#endif /* CF_UID */

/* return with whatever we have */

	return (rs >= 0) ? pwlen : rs ;
}
/* end subroutine (getpwlogname) */


