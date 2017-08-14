/* opensvc_quota */

/* LOCAL facility open-service (quota) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_GETUSERHOME	1		/* use 'getuserhome(3dam)' */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This code was started by taking the corresponding code from the
        TCP-family module. In retrospect, that was a mistake. Rather I should
        have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_quota(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/statvfs.h>
#include	<sys/fs/ufs_quota.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<char.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<localmisc.h>

#include	"opensvc_quota.h"
#include	"defs.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDUID	"MOTD_UID"

#define	NDEBFNAME	"opensvcfs_quota.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	statvfsdir(const char *,struct statvfs *) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	cfdeci(const char *,int,int *) ;

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;


/* local structures */


/* forward references */

static int	procuser(char *,int,const char *) ;
static int	mkfsline(char *,int,const char *,struct statvfs *) ;


/* local variables */


/* exported subroutines */


int opensvc_quota(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		argc = 0 ;
	int		pipes[2] ;
	int		fd = -1 ;
	int		ll = -1 ;
	const char	*un = NULL ;
	char		ubuf[USERNAMELEN+1] ;
	char		lbuf[LINEBUFLEN+1] ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	    if ((argc >= 2) && (argv[1] != '\0')) {
	        un = argv[1] ;
	    }
	}

/* default user as necessary */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_quota: 0 un=%s\n",un) ;
#endif

	if ((un == NULL) || (un[0] == '\0')) {
	    un = getourenv(envv,VARMOTDUSER) ;
	    if ((un == NULL) || (un[0] == '\0')) {
	        const char	*uidp = getourenv(envv,VARMOTDUID) ;
	        if ((uidp != NULL) && (uidp[0] != '\0')) {
	            int		v ;
	            uid_t	uid ;
	            rs = cfdeci(uidp,-1,&v) ;
	            uid = v ;
	            if (rs >= 0) {
	                un = ubuf ;
	                rs = getusername(ubuf,USERNAMELEN,uid) ;
	            }
	        }
	    }
	}
	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) un = "-" ;

/* process this user */

	if (rs >= 0) {
	rs = procuser(lbuf,llen,un) ;
	ll = rs ;
	}

/* write it out */

	if (rs >= 0) {
	if ((rs = u_pipe(pipes)) >= 0) {
	    int	wfd = pipes[1] ;
	    fd = pipes[0] ;

	    rs = u_write(wfd,lbuf,ll) ;

	    u_close(wfd) ;
	    if (rs < 0) u_close(fd) ;
	} /* end if (pipe) */
	} /* end if (ok) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_quota) */


/* local subroutines */


static int procuser(char *lbuf,int llen,cchar *un)
{
	const int	hlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		ll = 0 ;
	char		hbuf[MAXPATHLEN+1] ;

#if	CF_GETUSERHOME
	rs = getuserhome(hbuf,hlen,un) ;
#else
	{
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,un)) >= 0) {
	    	    strdcpy1(hbuf,hlen,pw.pw_dir) ;
		}
		uc_free(pwbuf) ;
	    } /* end if (memory-allocation) */
	}
#endif /* CF_GETUSERHOME */

	if (rs >= 0) {
	    struct statvfs	fss ;
	    if ((rs = statvfsdir(hbuf,&fss)) >= 0) {
	        rs = mkfsline(lbuf,llen,hbuf,&fss) ;
	        ll = rs ;
	    }
	} /* end if (have homedir) */

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (procuser) */


static int mkfsline(lbuf,llen,fpath,fssp)
char		lbuf[] ;
int		llen ;
const char	*fpath ;
struct statvfs	*fssp ;
{
	LONG		vt ;
	LONG		vavail ;
	LONG		vtotal ;
	double		per = -1.0 ;
	int		rs = SR_OK ;
	int		ll = 0 ;

	vt = fssp->f_bavail * fssp->f_frsize ;
	vavail = vt / 1024 ;

	vt = fssp->f_blocks * fssp->f_frsize ;
	vtotal = vt / 1024 ;

	vt = fssp->f_blocks - fssp->f_bavail ;
	if (fssp->f_blocks != 0) {
	    double	fn = ((double) (vt * 100)) ;
	    double	fd = ((double) fssp->f_blocks) ;
	    per = fn/fd ;
	}

/* we go through some trouble here to get a snughed-up floating number */

	{
	    const char	*dp ;
	    char	digbuf[DIGBUFLEN+1] ;
	    if (per < 0) per = 99.9 ;
	    rs = bufprintf(digbuf,DIGBUFLEN,"%4.1f",per) ;
	    if (rs >= 0) {
	        const char *fmt = "%s avail=%llu total=%llu util=%s%%\n" ;
	        dp = digbuf ;
	        while (dp[0] && CHAR_ISWHITE(*dp)) dp += 1 ;
	        rs = bufprintf(lbuf,llen,fmt,fpath,vavail,vtotal,dp) ;
	        ll = rs ;
	    }
	}

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (mkfsline) */


