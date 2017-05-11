/* uc_openpass */

/* interface component for UNIX® library-3c */
/* connect to a local program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a dialer to connect to a local program.

	Synopsis:

	int uc_openpass(passfname,oflags,timeout,opts)
	const char	passfname[] ;
	int		oflags ;
	int		timeout ;
	int		opts ;

	Arguments:

	passfname	the pass filename
	oflags		options to specify read-only or write-only
	timeout		time-out
	opts		options

	Returns:

	>=0		file descriptor to program STDIN and STDOUT
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	NOFILE
#define	NOFILE		20
#endif

#define	NENVS		100
#define	TO_CREAD	10

#ifndef	SHMNAMELEN
#define	SHMNAMELEN	MAXNAMELEN
#endif

#ifndef	DEBFNAME
#define	DEBFNAME	"child.deb"
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#define	DEFPATH		"/usr/bin:/bin"


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getpwd(char *,int) ;
extern int	dialpass(const char *,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* forward reference */

static int	accmode(int) ;


/* local variables */

enum accmodes {
	accmode_rdonly,
	accmode_wronly,
	accmode_rdwr,
	accmode_overlast
} ;


/* exported subroutines */


int uc_openpass(passfname,oflags,timeout,opts)
const char	passfname[] ;
int		oflags ;
int		timeout ;
int		opts ;
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("uc_openpass: ent passfname=%s\n",passfname) ;
#endif

	if (passfname == NULL)
	    return SR_FAULT ;

	if (passfname[0] == '\0')
	    return SR_INVALID ;

/* argument check */

	if ((rs = accmode(oflags)) >= 0) {
	    while ((passfname[0] == '/') && (passfname[1] == '/')) {
	        passfname += 1 ;
	    }
	    if ((rs = perm(passfname,-1,-1,NULL,(W_OK))) >= 0) {
	        rs = dialpass(passfname,timeout,opts) ;
	    }
	} /* end if (accmode) */

	return rs ;
}
/* end subroutine (uc_openpass) */


/* local subroutines */


static int accmode(int oflags)
{
	int		rs = SR_INVALID ;
	int		am = (oflags & (O_RDONLY | O_WRONLY | O_RDWR)) ;
	switch (am) {
	case (O_RDONLY):
	    rs = accmode_rdonly ;
	    break ;
	case (O_WRONLY):
	    rs = accmode_wronly ;
	    break ;
	case (O_RDWR):
	    rs = accmode_rdwr ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */
	return rs ;
}
/* end subroutine (accmode) */


