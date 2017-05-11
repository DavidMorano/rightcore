/* mktmpuserdir */

/* make a directory in the TMP-USER area */


#define	CF_DEBUGS	0		/* non-switchable */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This subroutine creates a directory that is specific to a user reserved
	directory inside the TMP directory.  For example, given a user named
	'fred' and a directory name 'junker' to create, the directory:

		/tmp/users/fred/junker/

	will be created.

	Synopsis:

	int mktmpuserdir(rbuf,un,dname,m)
	char		rbuf[] ;
	const char	un[] ;
	const char	dname[] ;
	mode_t		m ;

	where:

	rbuf		buffer to receive resulting created directory name
	un		optional username
	dname		basename of directory to create
	m		directory creation mode

	Returns:

	>0		length of resulting directory name
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<getxusername.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPUSERDNAME
#define	TMPUSERDNAME	"users"
#endif

#ifndef	TMPUSERDMODE
#define	TMPUSERDMODE	(S_IAMB | S_ISVTX)
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfocti(const char *,int,int *) ;


/* external variables */


/* local structures */


/* forward references */

static int	mktmpuser(char *,const char *) ;
static int	ensuremode(const char *,mode_t) ;


/* local variables */


/* exported subroutines */


int mktmpuserdir(char *rbuf,cchar *un,cchar *dname,mode_t m)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	char		unbuf[USERNAMELEN + 1] ;

	if (rbuf == NULL) return SR_FAULT ;
	if (dname == NULL) return SR_FAULT ;

	if (dname[0] == '\0') return SR_INVALID ;

	rbuf[0] = '\0' ;

	if ((un == NULL) || (un[0] == '\0') || (un[0] == '-')) {
	    un = unbuf ;
	    rs = getusername(unbuf,USERNAMELEN,-1) ;
	} /* end if */

	if (rs >= 0) {
	    char	tmpuserdname[MAXPATHLEN + 1] ;
	    if ((rs = mktmpuser(tmpuserdname,un)) >= 0) {
	        if ((rs = mkpath2(rbuf,tmpuserdname,dname)) >= 0) {
		    struct ustat	sb ;
	            len = rs ;
	            if ((rs = u_stat(rbuf,&sb)) >= 0) {
		        if (! S_ISDIR(sb.st_mode)) {
		            rs = SR_NOTDIR ;
			}
	            } else
	                rs = u_mkdir(rbuf,m) ;
	        } /* end if (mkpath) */
	    } /* end if (mktmpuser) */
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mktmpuserdir) */


/* local subroutines */


static int mktmpuser(char *rbuf,cchar *un)
{
	struct ustat	sb ;
	const int	tmpuserdmode = TMPUSERDMODE ;
	int		rs ;
	const char	*udname = TMPUSERDNAME ;
	const char	*tdname = getenv(VARTMPDNAME) ;
	char		tmpuserdname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("mktmpuserdir/mktmpuser: un=%s\n",un) ;
#endif

	if (tdname == NULL) tdname = TMPDNAME ;

	if ((rs = mkpath2(tmpuserdname,tdname,udname)) >= 0) {

	    if ((rs = u_stat(tmpuserdname,&sb)) == SR_NOEXIST) {
	        if ((rs = u_mkdir(tmpuserdname,tmpuserdmode)) >= 0) {
	            rs = ensuremode(tmpuserdname,tmpuserdmode) ;
	        }
	    }

	    if (rs >= 0) {
	        if ((rs = mkpath2(rbuf,tmpuserdname,un)) >= 0) {
	            if ((rs = u_stat(rbuf,&sb)) == SR_NOEXIST) {
	                rs = u_mkdir(rbuf,S_IAMB) ;
	            }
	        } /* end if (mkpath) */
	    } /* end if (ok) */

	} /* end if (ok) */

	return rs ;
}
/* end subroutine (mktmpuser) */


static int ensuremode(cchar *rbuf,mode_t nm)
{
	struct ustat	sb ;
	int		rs ;
	int		f = FALSE ;

	if (rbuf == NULL) return SR_FAULT ;

	if (rbuf[0] == '\0') return SR_INVALID ;

	if ((rs = u_open(rbuf,O_RDONLY,0666)) >= 0) {
	    const int	fd = rs ;

	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        mode_t	cm = sb.st_mode & (~ S_IFMT) ;

	        nm &= (~ S_IFMT) ;
	        if ((cm & nm) != nm) {
		    f = TRUE ;
	            nm |= cm ;
		    rs = u_fchmod(fd,nm) ;
	        }

	    } /* end if (stat) */

	    u_close(fd) ;
	} /* end if (open-close) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (ensuremode) */


