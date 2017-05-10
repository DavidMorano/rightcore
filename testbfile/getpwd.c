/* getpwd */

/* get current process directory */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine gets and returns, into a user supplied buffer, the
        current working directory maintained by UNIX®. The current working
        directory of the SHELL (maintained in the environment variable 'PWD') is
        returned if it is the same directory as that maintained by UNIX®.

	Synopsis:

	int getpwd(pwbuf,pwlen)
	char	pwbuf[] ;
	int	pwlen ;

	Arguments:

	pwbuf	- buffer to hold resultant path string
	pwlen	- buflen of buffer

	Returns:

	>=0	length of returned string in user buffer
	<0	BAD


	Note:

	On BSD systems, 'pipe(2)' does not open both ends of the pipe for
	both reading and writing, so we observe the old BSD behavior of
	the zeroth element FD being only open for reading and the oneth
	element FD only open for writing.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */

int	getpwds(struct ustat *,char *,int) ;


/* exported subroutines */


int getpwd(char *pwbuf,int pwlen)
{
	return getpwds(NULL,pwbuf,pwlen) ;
}
/* end subroutine (getpwd) */


int getpwds(struct ustat *sbp,char *pwbuf,int pwlen)
{
	struct ustat	*ssbp, sb1, sb2 ;
	int		rs = SR_NOENT ;
	int		rs1 ;
	int		pl = 0 ;
	const char	*pwd ;

#if	CF_DEBUGS
	debugprintf("getpwd: ent pwlen=%d\n",pwlen) ;
#endif

	if (pwbuf == NULL) return SR_FAULT ;

	if (pwlen < 0)
	    pwlen = MAXPATHLEN ;

	pwbuf[0] = '\0' ;

/* can we return with a quickie? */

	if ((pwd = getenv(VARPWD)) != NULL) {
	    if ((rs1 = u_stat(pwd,&sb1)) >= 0) {
		ssbp = (sbp != NULL) ? sbp : &sb2 ;
	        if ((rs = u_stat(".",ssbp)) >= 0) {
	            if ((sb1.st_dev == ssbp->st_dev) && 
	            	(sb1.st_ino == ssbp->st_ino)) {
		        rs = sncpy1(pwbuf,pwlen,pwd) ;
			pl = rs ;
		    } else {
		        rs = SR_NOENT ;
		    }
	        } /* end if (stat) */
	    } /* end if (stat) */
	} /* end if (quickie) */

/* continue with the hard way */

	if (rs == SR_NOENT) {
	    if ((rs = uc_getcwd(pwbuf,pwlen)) >= 0) {
	        pl = rs ;
	        if (sbp != NULL) {
		    rs = u_stat(pwbuf,sbp) ;
		}
	    }
	} /* end if (longer) */

#if	CF_DEBUGS
	debugprintf("getpwd: ret rs=%d pl=%u\n",rs,pl) ;
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (getpwds) */


