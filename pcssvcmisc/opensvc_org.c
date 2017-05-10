/* opensvc_org */

/* LOCAL facility open-service (org) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


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

	int opensvc_org(pr,prn,of,om,argv,envv,to)
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
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getxusername.h>
#include	<localmisc.h>

#include	"opensvc_org.h"
#include	"defs.h"


/* local defines */

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDUID	"MOTD_UID"

#define	NDEBFNAME	"opensvc_org.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	getuserorg(char *,int,const char *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isNotPresent(int) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int opensvc_org(pr,prn,of,om,argv,envv,to)
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
	int		cols = COLUMNS ;
	const char	*un = NULL ;
	const char	*varcols ;
	char		ubuf[USERNAMELEN+1] ;
	char		lbuf[LINEBUFLEN+2] ;

	varcols = getourenv(envv,VARCOLUMNS) ;
	if (varcols != NULL) {
	    int	rs1 ;
	    int	v ;
	    rs1 = cfdeci(varcols,-1,&v) ;
	    if (rs1 >= 0) cols = v ;
	}

/* find the username to act upon */

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	    if ((argc >= 2) && (argv[1] != '\0')) {
		un = argv[1] ;
	    }
	}

/* default user as necessary */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_org: 0 un=%s\n",un) ;
#endif

	if ((un == NULL) || (un[0] == '\0')) {
	    un = getourenv(envv,VARMOTDUSER) ;
	    if ((un == NULL) || (un[0] == '\0')) {
		const char	*uidp = getourenv(envv,VARMOTDUID) ;
		if ((uidp != NULL) && (uidp[0] != '\0')) {
		    int		v ;
		    uid_t	uid ;
		    if ((rs = cfdeci(uidp,-1,&v)) >= 0) {
		        uid = v ;
			un = ubuf ;
		        rs = getusername(ubuf,USERNAMELEN,uid) ;
		    }
		}
	    }
	}
	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) un = "-" ;

/* process this user */

	if (rs >= 0) {
	rs = getuserorg(lbuf,llen,un) ;
	ll = rs ;
	if (((rs >= 0) && (ll == 0)) || ((rs < 0) && isNotPresent(rs))) {
	    rs = localgetorg(pr,lbuf,llen,un) ;
	    ll = rs ;
	}
	if (rs >= 0) {
	    if (ll > cols) ll = cols ;
	    if (ll > 0) {
	        lbuf[ll++] = '\n' ;
	        lbuf[ll] = '\0' ;
	    }
	}
	} /* end if (ok) */

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

ret0:
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_org) */


