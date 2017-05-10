/* remotes */

/* subroutines to get to the remote machine */


#define	CF_DEBUG	0
#define	F_REXECL	1
#define	F_RCMDU		1


/* revision history:

	- David A.D. Morano, 96/11/21
	This program was started by copying from the RSLOW program.

	- David A.D. Morano, 96/12/12
	I modified the program to take the username and password
	from a specified file (for better security).


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>
#include	<stropts.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"netfile.h"


/* local defines */

#undef	CMDBUFLEN
#define	CMDBUFLEN	8192
#define	DISPLAYLEN	(MAXHOSTNAMELEN + 50)


/* external subroutines */

extern int	getnodedomain() ;
extern int	cfdec() ;
extern int	getchostname(), getehostname() ;
extern int	quoteshellarg(), qualdisplay() ;

extern char	*strshrink() ;
extern char	*strbasename() ;


/* forward subroutines */


/* external variables */

extern struct global	g ;

extern int		errno ;


/* local variables */





int rex_rexec(hostname,port,username,password,wip,cmd,fd2p)
char	hostname[] ;
int	port ;
char	username[], password[] ;
struct worm	*wip ;
char	cmd[] ;
int	*fd2p ;
{
	int	rs, rfd ;
	int	len ;

	char	buf[BUFLEN + 1] ;
	char	cmdbuf[(MAXPATHLEN * 2) + 1] ;
	char	ahostname[MAXHOSTNAMELEN + 1] ;
	char	*ahost ;


	if ((wip != NULL) && (wip->wfd >= 0)) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("rex_rexec: environment\n") ;
#endif

	    if ((rs = (int) lseek(wip->wfd,0L,SEEK_SET)) < 0)
	        rs = (- errno) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("rex_rexec: lseek rs=%d\n",rs) ;
#endif

	    if (rs < 0) return rs ;

	    sprintf(cmdbuf,
		"/bin/sh -c \"cp /dev/null %s ; /bin/cat > %s\"",
		wip->wormfname,
		wip->wormfname) ;

	    ahost = ahostname ;
	    strcpy(ahostname,hostname) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0) {

	        debugprintf("rex_rexec: calling REXECL\n") ;

	        debugprintf("rex_rexec: h=%s port=%d u=%s p=%s\n",
	            hostname,port,username,password) ;

	    }
#endif

#if	F_REXECL
	    rfd = rexecl(&ahost,(unsigned short) port,username,
	        password,cmdbuf,NULL) ;
#else
	    if (password == NULL) password = "" ;

	    rfd = rexec(&ahost,(unsigned short) port,username,
	        password,cmdbuf,NULL) ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("rex_rexec: REXECL rs=%d\n",rfd) ;
#endif

	    if (rfd < 0) return rfd ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("rex_rexec: continuing\n") ;
#endif

	    rs = 0 ;
	    while ((len = read(wip->wfd,buf,BUFLEN)) > 0) {

	        if ((rs = writen(rfd,buf,len)) < 0) break ;

	    }

	    if (len < 0) len = (- errno) ;

	    shutdown(rfd,2) ;

	    close(rfd) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("rex_rexec: REXECL len=%d rs=%d\n",len,rs) ;
#endif

	    if (len < 0) return len ;

	    if (rs < 0) return rs ;

	    cmd = cmdbuf ;
	    sprintf(cmdbuf,
	        "/bin/sh %s",
	        wip->wormfname) ;

#ifdef	BRAINDAMAGEWAIT
	if (BRAINDAMAGEWAIT > 0)
		sleep(BRAINDAMAGEWAIT) ;
#endif

	} /* end if (sending the worm over) */

	ahost = ahostname ;
	strcpy(ahostname,hostname) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("rex_rexec: calling REXECL 2\n") ;
#endif

#if	F_REXECL
	rs = rexecl(&ahost,(unsigned short) port,username,
	    password,cmd,fd2p) ;
#else
	if (password == NULL) password = "" ;

	rs = rexec(&ahost,(unsigned short) port,username,
	    password,cmd,fd2p) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("rex_rexec: REXECL 2 rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (rex_rexec) */



int rex_rcmd(hostname,username,wip,cmd,fd2p)
char	hostname[] ;
char	username[] ;
struct worm	*wip ;
char	cmd[] ;
int	*fd2p ;
{
	int	rs, rfd ;
	int	len ;

	char	buf[BUFLEN + 1] ;
	char	cmdbuf[(MAXPATHLEN * 2) + 1] ;


	if ((wip != NULL) && (wip->wfd >= 0)) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("rex_rcmd: environment\n") ;
#endif

	    if ((rs = (int) lseek(wip->wfd,0L,SEEK_SET)) < 0)
	        rs = (- errno) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("rex_rcmd: lseek rs=%d\n",rs) ;
#endif

	    if (rs < 0) return rs ;

	    sprintf(cmdbuf,"/bin/cat > %s",wip->wormfname) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("rex_rcmd: calling RCMDU\n") ;
#endif

	    rfd = rcmdu(hostname,username,
	        cmdbuf,NULL) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("rex_rcmd: RCMDU 2 rs=%d\n",rfd) ;
#endif

	    if (rfd < 0) return rfd ;

	    rs = 0 ;
	    while ((len = read(wip->wfd,buf,BUFLEN)) > 0) {

	        if ((rs = writen(rfd,buf,len)) < 0) break ;

	    }

	    if (len < 0) len = (- errno) ;

	    shutdown(rfd,2) ;

	    close(rfd) ;

	    if (len < 0) return len ;

	    if (rs < 0) return rs ;

	    cmd = cmdbuf ;
	    sprintf(cmdbuf,"/bin/sh %s",wip->wormfname) ;

#ifdef	BRAINDAMAGEWAIT
	if (BRAINDAMAGEWAIT > 0)
		sleep(BRAINDAMAGEWAIT) ;
#endif

	} /* end if (sending the worm over) */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("rex_rcmd: calling RCMDU 2\n") ;
#endif

	rs = rcmdu(hostname,username,
	    cmd,fd2p) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("rex_rcmd: RCMDU 2 rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (rex_rcmd) */



