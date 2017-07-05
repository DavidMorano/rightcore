/* rfile */

/* subroutines to write to remote files or read from them */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	- 1996-11-21, Dave morano
        This subroutine code was started by copying from some other program (one
        of the other PCS remote host access programs or subroutines).

*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

        The subroutine either returns a FD for the remote file or it returns an
        error which is indicated by a negative valued return.

        Depending on the arguments to the subroutine call, both the INET 'exec'
        or 'shell' services may be invoked to try and make a connection to the
        remote host.

	Synopsis:

	int rfile(rhost,auth,rfilename,flags,mode)
	const char	rhost[] ;
	const char	rfilename[] ;
	int		flags, mode ;
	struct rex_auth {
		char	*restrict ;
		char	*username ;
		char	*password ;
		NETFILE_ENT	**machinev ;
	} *auth ;

	Arguments:

	Returns:


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/utsname.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<netfile.h>
#include	<localmisc.h>

#include	"rex.h"
#include	"incfile_rfilewrite.h"


/* local defines */

#define	PROG_SHELL	"/bin/sh"

#define	BUFLEN		(8 * 1024)
#define	CMDBUFLEN	8192


/* external subroutines */

extern int	reade() ;
extern int	getnodedomain() ;
extern int	getchostname(), getehostname() ;

extern char	*strbasename() ;


/* forward subroutines */

static int	hostequiv() ;


/* external variables */


/* local variables */


/* exported subroutines */


int rfile(rhost,auth,rfilename,flags,mode)
const char	rhost[] ;
const char	rfilename[] ;
REX_AUTH	*auth ;
int		flags, mode ;
{
	REX_FL		f ;
	REX_AUTH	aa, *ap ;
	NETFILE_ENT	*mp ;
	int		i, j ;
	int		srs, rs, len, l ;
	int		fd, fd2 ;
	const char	*prog_shell = PROG_SHELL ;
	const char	*args[4] ;

	char	*cp, *cp1, *cp2 ;

	char	buf[BUFLEN + 1], *bp ;
	char	hostname[BUFLEN + 1] ;
	char	jobfname[MAXPATHLEN + 1], *jobid ;


	if ((rhost == NULL) || (rhost[0] == '\0'))
	    goto badhost ;

	if ((rfilename == NULL) || (rfilename[0] == '\0'))
	    goto badfile ;


/* make a job file name */

	if ((rs = mkjobfile("/tmp",0600,jobfname)) < 0)
	    return SR_PROTO ;

	jobid = strbasename(jobfname) ;

/* start by opening a connection to the remote machine */

#if	CF_DEBUGS
	debugprintf("rfile: creating remote command\n") ;
#endif

	f.keepalive = FALSE ;
	if (flags & O_KEEPALIVE) 
		f.keepalive = TRUE ;

	bufprintf(buf,BUFLEN,"/bin/cat > %s",jobfname) ;

	args[0] = "sh" ;
	args[1] = "-c" ;
	args[2] = buf ;
	args[3] = NULL ;
	fd = rex(rhost,auth,&f,prog_shell,args,NULL,&mp) ;

#if	CF_DEBUGS
	debugprintf("rfile: REX rs=%d\n",fd) ;
#endif

	if (fd < 0) {

	    u_unlink(jobfname) ;

	    return fd ;
	}

/* shutdown further reception on the socket */

	u_shutdown(fd,0) ;

	srs = fd ;

/* we have a connection, let's try to make the best use of it ! */

	if ((flags & O_WRONLY) || (flags & O_APPEND)) {

#if	CF_DEBUGS
	    debugprintf("rfile: writing only (or append)\n") ;
#endif

	    bp = buf ;
	    len = 0 ;

/* send over a greeting ! */

	    len += sprintf(buf + len," # <-- force CSH to use Bourne\n\n") ;

	    len += sprintf(buf + len,"# RFILE job\n\n") ;

	    len += sprintf(buf + len,"JOBFNAME=%s\n",jobfname) ;

/* send the file name over */

	    len += sprintf(buf + len,"FILE=%s\n",rfilename) ;

/* send over the "open" flags */

	    len += sprintf(buf + len,"F_C=%s\n",
	        (flags & O_CREAT) ? "true" : "false") ;

	    len += sprintf(buf + len,"F_T=%s\n",
	        (flags & O_TRUNC) ? "true" : "false") ;

	    len += sprintf(buf + len,"F_A=%s\n",
	        (flags & O_APPEND) ? "true" : "false") ;

/* send the file creation mode over */

	    len += sprintf(buf + len,"MODE=%4o\n",
	        mode & 0777) ;

/* send it all over there */

#if	CF_DEBUGS
	    debugprintf("rfile: worm length=%d\n",len) ;
#endif

	    l = 0 ;
	    while (((len - l) > 0) && ((rs = u_write(fd,buf + l,len - l)) > 0))
	        l += rs ;

#if	CF_DEBUGS
	    debugprintf("rfile: back from 'write' w/ rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        srs = rs ;

/* send over the "write" program */

	    if (srs >= 0) {

#if	CF_DEBUGS
	        debugprintf("rfile: sending over the worm\n") ;
#endif

	        len = INCFILELEN_rfilewrite ;
	        bp = (char *) incfile_rfilewrite ;
	        l = 0 ;
	        while (((len - l) > 0) && 
	            ((rs = u_write(fd,bp + l,len - l)) > 0))
	            l += rs ;

	        if (rs < 0)
	            srs = rs ;

#if	CF_DEBUGS
	        debugprintf("rfile: sent worm w/ srs=%d\n",srs) ;
#endif

	    } /* end if */

	    close(fd) ;

	} else {

#if	CF_DEBUGS
	    debugprintf("rfile: unimplemented file access mode\n") ;
#endif

	    close(fd) ;

	    srs = SR_PROTO ;
	}

/* try to execute the worm */

#if	CF_DEBUGS
	debugprintf("rfile: thinking about executing the worm\n") ;
#endif

	fd = -1 ;
	if (srs >= 0) {

/* can we arrange for a short-cut for the REX connection? */

	    ap = auth ;
	    if ((ap != NULL) && (mp != NULL)) {

#if	CF_DEBUGS
	        debugprintf("rfile: had authorization & NETRC entry\n") ;
#endif

	        memcpy(&aa,ap,sizeof(struct rex_auth)) ;

#if	CF_DEBUGS
	        debugprintf( "rfile: made AUTH copy\n") ;
#endif

	        aa.restrict = "rcmd" ;
	        if (mp->login != NULL)
	            aa.username = mp->login ;

#if	CF_DEBUGS
	        debugprintf( "rfile: made AUTH login copy\n") ;
#endif

	        if (mp->password != NULL)
	            aa.password = mp->password ;

#if	CF_DEBUGS
	        debugprintf( "rfile: made AUTH password copy\n") ;
#endif

	        ap = &aa ;

	    } /* end if (changing the authorization) */

#if	CF_DEBUGS
	    debugprintf( "rfile: about to execute REX (again)\n") ;
#endif

	    args[0] = "rfile" ;
	    args[1] = jobfname ;
	    args[2] = NULL ;
	    fd = rex(rhost,ap,&f,"/bin/sh",args,&fd2,NULL) ;

#if	CF_DEBUGS
	    debugprintf("rfile: REX rs=%d\n",fd) ;
#endif

	    if (fd < 0) {

	        u_unlink(jobfname) ;

	        return fd ;
	    }

	    srs = fd ;

/* check if we have the go ahead signal from the other end */

#if	CF_DEBUGS
	    debugprintf("rfile: check for signal from other end\n") ;
#endif

	    len = 0 ;
	    while (((l = reade(fd2,buf + len,1,FM_NONE,15)) > 0)
	        && (buf[len] != '\n'))
	        len += l ;

#if	CF_DEBUGS
	    debugprintf("rfile: out of signal check 1, l=%d len=%d buf=%W\n",
	        l,len,buf,len) ;
#endif

	    if (l < 0)
	        srs = l ;

	    else if ((len < 1) || (strncmp(buf,"OK",2) != 0))
	        srs = SR_ACCES ;

#if	CF_DEBUGS
	    debugprintf("rfile: out of signal check, srs=%d\n",srs) ;
#endif

	} /* end if (we attempted to execute the worm) */

#if	CF_DEBUGS
	debugprintf("rfile: finishing up\n") ;
#endif

	u_close(fd2) ;

	if ((srs < 0) && (fd >= 0)) 
		u_close(fd) ;

	u_unlink(jobfname) ;

	return srs ;

/* bad returns come here */
badret:
	return srs ;

badhost:
	srs = SR_INVAL ;
	goto badret ;

badfile:
	srs = SR_INVAL ;
	goto badret ;

}
/* end subroutine (rfile) */


