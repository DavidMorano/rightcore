/* worm */

/* subroutine to make the worm to be remotely executed */
/* last modified %G% version %I% */


#define	CF_DEBUG	1


/* revision history:

	= David A.D. Morano, November 1995

	This subroutine was originally written.

	= David A.D. Morano, 97/04/16

	This subroutine was adopted from the RSHE program.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Call as :

	worm() ;


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/wait.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<rpc/rpc.h>
#include	<rpcsvc/rstat.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* defines */

#undef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + (LINELEN * 2))
#define	DISBUFLEN	300
#define	LOGNAMELEN	40


/* external subroutines */

extern int	getlogname() ;
extern int	cfdec() ;
extern int	matstr() ;
extern int	getnodedomain() ;
extern int	mkjobfile(), unlinkd() ;
extern int	quotevalue() ;
extern int	send_rxenv() ;
extern int	send_procenv() ;

extern char	*putheap() ;
extern char	*strbasename() ;
extern char	*timestr_log() ;


/* forward references */


/* external variables */

extern struct global	g ;

extern struct userinfo	u ;


/* global data */


/* local data structures */


/* exported subroutines */


int worm_init(wip,ruser,envfile,f_x,f_d,rxport,rcmd)
struct worm	*wip ;
char	ruser[] ;
char	envfile[] ;
int	f_x, f_d ;
char	rxport[] ;
char	rcmd[] ;
{
	bfile		wormfile, *wfp = &wormfile ;

	struct jobinfo	ji ;

	int	i ;
	int	len, l, rs ;
	int	n ;
	int	fd ;

	char	*cwd = NULL ;
	char	buf[BUFLEN + 1] ;
	char	*cp ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("worm_init: about to create the jobfile\n") ;
#endif

	wip->wormfname[0] = '\0' ;
	if ((rs = mkjobfile("/tmp",0740,wip->wormfname)) < 0)
	    goto badjobfile ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("worm_init: wormfname=%s\n",wip->wormfname) ;
#endif

	if ((rs = bopen(wfp,wip->wormfname,"rwct",0744)) < 0)
	    goto badjobopen ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("worm_init: opened the wormfname\n") ;
#endif


	wip->f_delete = FALSE ;
	if (unlinkd(wip->wormfname,LOCKTIMEOUT) < 0)
	    wip->f_delete = TRUE ;


/* put some stuff together for later */

	wip->wfd = -1 ;
	bcontrol(wfp,BC_FD,&fd) ;

	wip->wfd = dup(fd) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("worm_init: wfd=%d\n",wip->wfd) ;
#endif


/* start to write out !!! */

	bprintf(wfp," # <-- force CSH to use Bourne shell\n") ;

	bprintf(wfp,"# %s REX %s/%s\n",
	    timestr_log(g.daytime,buf),
	    VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	bprintf(wfp,"# %s!%s\n",u.nodename,u.username) ;

	bprintf(wfp,"# ruser=%s\n",ruser) ;

	bprintf(wfp,"\n") ;


/* execute some stuff on the remote which is supposed to be there already */

	wip->offset = -1 ;
	if ((ruser != NULL) && (ruser[0] != '\0')) {

	    bprintf(wfp,"LOGNAME=") ;

	    wip->offset = bseek(wfp,0L,SEEK_CUR) ;

	    bprintf(wfp,"%-40s\n",ruser) ;

	    bprintf(wfp,"export LOGNAME\n") ;

	} else
	    bprintf(wfp,": ${LOGNAME:=${USER}}\nexport LOGNAME\n") ;


/* have the remote side execute the environment file if it exists */

	if (envfile[0] != '\0') {

	    if (g.debuglevel > 0) {

	        bprintf(g.efp,"%s: envfile=\"%s\"\n",
	            g.progname,envfile) ;

	    }

	    if (g.f.log)
	        logfile_printf(&g.lh,"envfile=\"%s\"\n",
	            envfile) ;

	    bprintf(wfp,"if [ -r %s%s ] ; then\n",
	        (envfile[0] != '/') ? "${HOME}/" : "",envfile) ;

	    bprintf(wfp,". %s%s < /dev/null > /dev/null 2>&1\n",
	        (envfile[0] != '/') ? "${HOME}/" : "",envfile) ;

	    bprintf(wfp,"fi\n") ;

	} /* end if (processing the remote environment file) */


/* OK, now we send over all (essentially) or some of our local environemnt */


/* put some stuff together for later */

	ji.f_remotedomain = g.f.remote ;
	ji.nodename = u.nodename ;
	ji.domainname = u.domainname ;
	ji.jfp = wfp ;



	if (f_x && (rxport == NULL)) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("worm_init: sending PROCESS envioronment\n") ;
#endif

	    send_procenv(&ji) ;

	} else {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("worm_init: sending RXPORT environment\n") ;
#endif

	    send_rxenv(&ji,rxport) ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("worm_init: sent over the environment\n") ;
#endif


/* OK, now send over the stuff that is fixed */

	bprintf(wfp,"RUNMODE=rcmd\nexport RUNMODE\n") ;

	if (g.f.remote)
	    bprintf(wfp,"RCMD_MACHINE=%s.%s\n",
	        u.nodename, u.domainname) ;

	else
	    bprintf(wfp,"RCMD_MACHINE=%s\n",
	        u.nodename) ;

	bprintf(wfp,"export RCMD_MACHINE\n") ;

	bprintf(wfp,"RCMD_USER=%s\nexport RCMD_USER\n",
	    u.username) ;


/* should we arrange for a change of directory on the remote ? */

	cwd = NULL ;
	if (f_d) {

	    if (((cp = getenv("PWD")) != NULL) && (access(cp,X_OK) == 0))
	        cwd = putheap(cp) ;

	    if (cwd == NULL)
	        cwd = getcwd(NULL,0) ;

	    if (cwd != NULL) {

	        if (g.debuglevel > 0) {

	            bprintf(g.efp,"%s: CHDIR=%s\n",
	                g.progname,cwd) ;

	        }

	        if (g.f.log) {

	            logfile_printf(&g.lh,"CHDIR=%s\n",
	                cwd) ;

	        }

	        bprintf(wfp,"if [ -d %s -a -x %s ] ; then\n",cwd,cwd) ;

	        bprintf(wfp,"cd %s 2> /dev/null\n",cwd) ;

	        bprintf(wfp,"else\n") ;

	        bprintf(wfp,"exit 128\n") ;

	        bprintf(wfp,"fi\n") ;

	    } /* end if (we could get a CWD) */

	} /* end if (arranging for a change of directory on remote) */

/* set the PWD environment variable properly */

	if (cwd == NULL) {

	    bprintf(wfp,"if [ -z \"${PWD}\" ] ; then\n") ;

	    bprintf(wfp,"PWD=${HOME}\nexport PWD\n") ;

	    bprintf(wfp,"fi\n") ;

	} else 
	    bprintf(wfp,"PWD=%s\nexport PWD\n",cwd) ;


/* get ready to boogie */

	if (g.f.verbose || (g.debuglevel > 0)) {

	    bprintf(g.efp,"%s: wormfname=\"%s\"\n",
	        g.progname,wip->wormfname) ;

	}

	if (g.f.log)
	    logfile_printf(&g.lh,"worm jobid=%s\n",
	        strbasename(wip->wormfname)) ;

	bprintf(wfp,
	    "/bin/rm -f %s\n",
	    wip->wormfname) ;

	if ((rcmd == NULL) || (rcmd[0] == '\0'))
	    bprintf(wfp,"exec /bin/ksh -i\n") ;

	else
	    bprintf(wfp,"exec %s\n",rcmd) ;

	bprintf(wfp,"exit 129\n") ;

	bprintf(wfp,"\n") ;

	if ((rs = bclose(wfp)) < 0) goto badjobclose ;


	return OK ;

/* handle the bad stuff */
badjobfile:
	bprintf(g.efp,"%s: couldn't create a TMP file\n",
	    g.progname) ;

	return rs ;

badjobopen:
	unlink(wip->wormfname) ;

	bprintf(g.efp,"%s: couldn't open a TMP file\n",
	    g.progname) ;

	return rs ;

badjobclose:
	unlink(wip->wormfname) ;

	bprintf(g.efp,"%s: couldn't close a TMP file\n",
	    g.progname) ;

	return rs ;

}
/* end subroutine (worm_init) */


/* update the logname in the worm file */

static char	blanks[] = "                                        " ;

int worm_update(wip,logname,len)
struct worm	*wip ;
char		logname[] ;
int		len ;
{
	int	rs ;

	char	userbuf[42] ;


	if (wip == NULL) return SR_OK ;

#if	CF_DEBUG
	if (g.debuglevel > 2) {

	    logname[32] = '\0' ;
	    debugprintf("worm_update: entered wfd=%d logname=%s\n",
	        wip->wfd,logname) ;

	}
#endif

	if ((wip->offset < 0) || (wip->wfd < 0)) return SR_BAD ;

#if	CF_DEBUG
	if (g.debuglevel > 2)
	    debugprintf("worm_update: got offset\n") ;
#endif

	if ((rs = lseek(wip->wfd,wip->offset,SEEK_SET)) < 0) rs = (- errno) ;

	if (rs >= 0) {

	    if (len < 0) len = strlen(logname) ;

	    strncpy(userbuf,logname,len) ;

	    memcpy(userbuf + len,blanks,LOGNAMELEN - len) ;

	    if ((rs = write(wip->wfd,userbuf,LOGNAMELEN)) < 0) rs = (- errno) ;

	}

	return rs ;
}
/* end subroutine (worm_update) */


/* free up the worm resources */
int worm_free(wip)
struct worm	*wip ;
{
	int	rs ;


	if (wip == NULL) return SR_BAD ;

	if (wip->wfd < 0) return SR_BAD ;

	if ((rs = close(wip->wfd)) < 0) rs = (- errno) ;

	unlink(wip->wormfname) ;

	return rs ;
}
/* end subroutine (worm_free) */



