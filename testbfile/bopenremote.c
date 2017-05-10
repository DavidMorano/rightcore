/* bopenremote */

/* execute a command remotely */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	- 1998-10-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Open (within the BFILE framework) something.

*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	BUFLEN		(MAXPATHLEN + (LINELEN * 2))
#define	DISBUFLEN	300


/* external subroutines */

extern int	mkjobfile(const char *,mode_t,char *) ;

extern char	*strbasename(char *) ;


/* forward references */

static int	quotevalue(const char *,char *,int,const char **) ;
static int	newbuf() ;


/* global data */


/* local structures */


/* exported subroutines */


int bopenremote(fpa,environ,remotehost,cmd)
bfile	*fpa[] ;
char	*environ[] ;
char	remotehost[] ;
char	cmd[] ;
{
	bfile	jobfile, *jfp = &jobfile ;

	struct utsname		uts ;

	pid_t	pid ;

	int	rs = SR_BAD ;
	int	i ;
	int	klen ;
	int	f_cwd = FALSE ;
	int	f_ksh = FALSE ;

	const char	*vp, *nvp ;

	char	remotecmd[(MAXPATHLEN * 2) + 1] ;
	char	jobfname[MAXPATHLEN + 1] ;
	char	*cwd = NULL ;
	char	**ep ;
	char	*nodename, *domainname ;
	char	valuebuf[MAXPATHLEN + 1] ;
	char	displaybuf[DISBUFLEN + 1] ;
	char	*cp ;
	char	*cmd_shell = NULL ;
	char	*cmd_rcp, *cmd_rsh ;


#if	CF_DEBUGS
	debugprintf("bopenremote: entered host=%s cmd>\n%s\n",
	    remotehost,cmd) ;
#endif

	if (fpa == NULL)
		return SR_FAULT ;

	jobfname[0] = '\0' ;
	if ((remotehost == NULL) || (remotehost[0] == '\0'))
	    goto badhost ;

	if ((cmd == NULL) || (cmd[0] == '\0'))
	    goto badhost ;

/* OK so far, get some command names */

	cmd_rsh = "/bin/rsh" ;
	if (u_access(cmd_rsh,X_OK) != 0) {

	    cmd_rsh = "/usr/ucb/rsh" ;
	    if (u_access(cmd_rsh,X_OK) != 0) 
		return SR_BAD ;

	}

	cmd_rcp = "/bin/rcp" ;
	if (u_access(cmd_rcp,X_OK) != 0) {

	    cmd_rcp = "/usr/ucb/rcp" ;
	    if (u_access(cmd_rcp,X_OK) != 0) 
		return SR_BAD ;

	}

/* get our host (nodename) and domain name */

	u_uname(&uts) ;

	nodename = uts.nodename ;
	domainname = NULL ;
	if ((cp = strchr(uts.nodename,'.')) != NULL) {

	    *cp++ = '\0' ;
	    domainname = cp ;
	}

	if (domainname == NULL) {

	    if (strncmp(nodename,"ho",2) == 0) {
	        domainname = "info.att.com" ;

	    } else if (strncmp(nodename,"mt",2) == 0) {
	        domainname = "mt.att.com" ;

	    } else if (strncmp(nodename,"dr",2) == 0) {
	        domainname = "dr.att.com" ;

	    } else if (strncmp(nodename,"lz",2) == 0)
	        domainname = "lz.att.com" ;

	} /* end if */

/* get what SHELL we will be using */

	if ((cmd_shell = getenv("SHELL")) == NULL) {

	    f_ksh = TRUE ;
	    cmd_shell = "/bin/ksh" ;
	}

#if	CF_DEBUGS
	cmd_shell = "/home/dam/src/runadvice_1/ksh2" ;
	debugprintf("bopenremote: SHELL=%s\n",cmd_shell) ;
#endif

/* what SHELL to use on the remote side ? */

	if ((! f_ksh) || (strcmp(strbasename(cmd_shell),"ksh") == 0))
	    f_ksh = TRUE ;

#if	CF_DEBUGS
	debugprintf("bopenremote: f_ksh=%d\n",f_ksh) ;
#endif

/* put together the remote command file */

	jobfname[0] = '\0' ;
	if ((rs = mkjobfile("/tmp",0740,jobfname)) < 0) 
		goto badjobfile ;

	if ((rs = bopen(jfp,jobfname,"wct",0744)) < 0) 
		goto badjobopen ;

#if	CF_DEBUGS
	debugprintf("bopenremote: jobfname=%s\n",jobfname) ;
#endif

/* write out our environment to the remote command */

	ep = environ ;
	for (i = 0 ; ep[i] != NULL ; i += 1) {

	    int	f_alloc ;


#if	CF_DEBUGS
	    debugprintf("bopenremote: top environment loop\n") ;
#endif

	    if ((ep[i][0] == '_') ||
	        (ep[i][0] < 32)) continue ;

/* break it up into the two pieces */

	    f_alloc = FALSE ;
	    vp = "" ;
	    if ((cp = strchr(ep[i],'=')) != NULL) {

	        klen = cp - ep[i] ;	/* length of variable name */

#if	CF_DEBUGS
	        debugprintf("bopenremote: environment name=%W\n",ep[i],klen) ;
#endif

/* process the value part */

	        vp = ep[i] + klen + 1 ;

/* perform some preprocessing on specific variables */

	        if (strncmp(ep[i],"DISPLAY",klen) == 0) {

	            if (*vp == ':') {

#if	CF_DEBUGS
	                debugprintf("bopenremote: special DISPLAY\n") ;
#endif

	                sprintf(displaybuf,"%s%s%s%s", nodename,
	                    (domainname != NULL) ? "." : "",
	                    (domainname != NULL) ? domainname : "",
	                    vp) ;

	                vp = displaybuf ;
	            }

	        } /* end if (we have a DISPLAY variable) */

/* perform general character escape processing */

#if	CF_DEBUGS
	        debugprintf("bopenremote: about to call quote\n") ;
#endif

	        if ((rs = quotevalue(vp,valuebuf,MAXPATHLEN,&nvp)) < 0) {

#if	CF_DEBUGS
	            debugprintf("bopenremote: return bad alloc rs=%d\n",rs) ;
#endif

	            goto badalloc ;

	        }

#if	CF_DEBUGS
	        debugprintf("bopenremote: returned w/ rs=%d\n",rs) ;
#endif

	        f_alloc = rs ;
	        if (f_alloc) 
			vp = nvp ;

	        if (klen > 0) {

	            if (f_ksh) {

	                bprintf(jfp,"export %W=\"%s\"\n",
	                    ep[i],klen, vp) ;

	            } else {

	                bprintf(jfp,"%W=\"%s\"\n",
	                    ep[i],klen, vp) ;

	                bprintf(jfp,"export %W\n",
	                    ep[i],klen) ;

	            }

	        } /* end if (we have something to send over) */

	        if (f_alloc) 
			uc_free(vp) ;

	    } /* end if (we have a value to this variable) */

	} /* end for (sending over environment) */

#if	CF_DEBUGS
	debugprintf("bopenremote: sent over the environment\n") ;
#endif

	bprintf(jfp,"RUNMODE=rcmd\nexport RUNMODE\n") ;

/* execute the remote command */

	if ((cwd = getenv("PWD")) == NULL) {

	    f_cwd = TRUE ;
	    cwd = (char *) getcwd(NULL,0) ;

	}

	if (cwd != NULL)
	    bprintf(jfp,"cd %s\n",cwd) ;

	bprintf(jfp,"rm -f %s\n",jobfname) ;

#ifdef	COMMENT
	bprintf(jfp,"echo %s >> /home/dam/rje/bor_log\n",cmd) ;

	bprintf(jfp,"echo hello from inside\n") ;
#endif

	bprintf(jfp,"exec %s\n",cmd) ;

	if ((rs = bclose(jfp)) < 0) 
		goto badhost2 ;

/* put the job command file on the remote host */

	sprintf(remotecmd,"%s %s %s:%s",
	    cmd_rcp,jobfname,remotehost,jobfname) ;

	if ((rs = system(remotecmd)) != 0) 
		goto badhost2 ;

/* execute the job command file on the remote host */

	sprintf(remotecmd,"%s %s %s %s",
	    cmd_rsh,remotehost,cmd_shell,jobfname) ;

#if	CF_DEBUGS
	debugprintf("bopenremote: remotecmd>\n%s\n",remotecmd) ;
#endif

	pid = (pid_t) bopencmd(fpa,remotecmd) ;

#if	CF_DEBUGS
	debugprintf("bopenremote: spawned pid=%d\n",pid) ;
#endif

/* clean up */

	if (f_cwd) 
		uc_free(cwd) ;

	if (strcmp(nodename,remotehost) != 0)
	    u_unlink(jobfname) ;

	return ((int) pid) ;

/* bad returns */
badhost2:
	if (f_cwd && (cwd != NULL)) {
	    uc_free(cwd) ;
	    cwd = NULL ;
	}

badalloc:
badjobopen:
	u_unlink(jobfname) ;

badjobfile:
badret:

#if	CF_DEBUGS
	debugprintf("bopenremote: ret rs=%d\n",rs) ;
#endif

	return rs ;

badhost:
	rs = SR_INVALID ;
	goto badret ;
}
/* end subroutine (bopenremote) */


/* local subroutines */


#ifdef	COMMENT

static void fixdisplay(host)
char	host[] ;
{
	struct utsname		uts ;

	int	rs1 ;
	int	size ;
	int	len, dlen ;

	char	*hp ;
	char	*cp, *cp2 ;


/* fix up the environment variable DISPLAY, if found */

	if ((cp = getenv("DISPLAY")) == NULL) 
		return ;

/* is an adjustment needed ? */

	if (cp[0] == ':') {

	    hp = host ;
	    if ((host == NULL) || (host[0] == '\0')) {

	        uname(&uts) ;

	        if ((cp2 = strchr(uts.nodename,'.')) != NULL)
	            *cp2 = '\0' ;

	        hp = uts.nodename ;

	    }

	    len = strlen(hp) ;

	    dlen = (len + 30) ;
	    size = (dlen + 1) ;
	    rs1 = uc_malloc(size,&cp2) ;

	    if (rs1 >= 0) {

	    bufprintf(cp2,dlen,"DISPLAY=%s%s",hp,cp) ;

	    putenv(cp2) ;

	    }

	}

}
/* end subroutine (fixdisplay) */

#endif /* COMMENT */


static int quotevalue(vs,buf,buflen,nvpp)
const char	vs[] ;
char		buf[] ;
int		buflen ;
const char	**nvpp ;
{
	int	rs = SR_OK ;
	int	mlen ;
	int	curbuflen = buflen ;
	int	newbuflen ;
	int	blen ;
	int	rlen = 0 ;
	int	f_got = FALSE ;
	int	f_malloc = FALSE ;

	const char	*tp, *cp ;

	char	*curbuf = buf ;


#if	CF_DEBUGS
	debugprintf("bopenremote: entered value=%s\n",vs) ;
#endif

	cp = vs ;
	rlen = buflen ;
	blen = 0 ;
	while ((tp = strpbrk(cp,"\\\"")) != NULL) {

#if	CF_DEBUGS
	    debugprintf("bopenremote: got one\n") ;
#endif

	    f_got = TRUE ;
	    mlen = tp - cp ;
	    if (mlen > rlen) {

	        if ((newbuflen = 
	            newbuf(curbuf,curbuflen,f_malloc,&curbuf)) < 0)
	            goto badalloc ;

	        f_malloc = TRUE ;
	        rlen += (newbuflen - curbuflen) ;
	        curbuflen = newbuflen ;
	    }

	    memcpy(curbuf + blen,cp,mlen) ;

	    blen += mlen ;
	    curbuf[blen++] = '\\' ;
	    curbuf[blen++] = *tp++ ;
	    rlen -= (mlen + 2) ;
	    cp = tp ;

#if	CF_DEBUGS
	    debugprintf("bopenremote: bottom of got one\n") ;
#endif

	} /* end while */

	if (f_got) {

#if	CF_DEBUGS
	    debugprintf("bopenremote: after got one\n") ;
#endif

	    mlen = strlen(cp) ;

	    if (mlen > rlen) {

	        if ((newbuflen = 
	            newbuf(curbuf,curbuflen,f_malloc,&curbuf)) < 0)
	            goto badalloc ;

	        f_malloc = TRUE ;
	        rlen += (newbuflen - curbuflen) ;
	        curbuflen = newbuflen ;
	    }

	    memcpy(curbuf,cp,mlen) ;
	    blen += mlen ;
	    rlen -= mlen ;
	    curbuf[blen] = '\0' ;

#if	CF_DEBUGS
	    debugprintf("bopenremote: bottom after got one\n") ;
#endif

	} /* end if (we got something) */

#if	CF_DEBUGS
	debugprintf("bopenremote: exiting f_got=%d\n",f_got) ;
#endif

	*nvpp = (f_got) ? curbuf : vs ;

ret0:

#if	CF_DEBUGS
	debugprintf("bopenremote: exiting for real, blen=%d\n",blen) ;
#endif

	return (rs >= 0) ? blen : rs ;

badalloc:
	if (f_malloc) 
	    uc_free(curbuf) ;

	rs = SR_NOMEM ;
	goto ret0 ;
}
/* end subroutine (quotevalue) */


static int newbuf(curbuf,curbuflen,f,nbpp)
int	curbuflen, f ;
char	*curbuf ;
char	**nbpp ;
{
	int	rs ;
	int	newbuflen = 0 ;

	caddr_t	p ;


	if (f) {

	    newbuflen = curbuflen * 2 ;
	    rs = uc_realloc(curbuf,newbuflen,&p) ;

	} else {

	    newbuflen = curbuflen * 2 ;
	    rs = uc_malloc(newbuflen,&p) ;

	}

	if (rs >= 0)
	    *nbpp = (char *) p ;

	return (rs >= 0) ? newbuflen : rs ;
}
/* end subroutine (newbuf) */



