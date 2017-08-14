/* bopenrcmde */

/* execute a command remotely */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	- 1998-10-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is supposed to be the same as the 'bopenrcmd(3b)' subroutine
	except that environment can be passed to the remote command.


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	LINELEN		200
#define	BUFLEN		(MAXPATHLEN + (LINELEN * 2))
#define	DISBUFLEN	300


/* external subroutines */

extern int	getnodedomain(char *,char *) ;
extern int	mkjobfile(const char *,mode_t,char *) ;

extern char	*strbasename(char *) ;


/* forward references */

static int	quotevalue() ;
static int	newbuf() ;


/* local data structures */


/* local module data */


/* exported subroutines */


int bopenrcmde(fpa,environ,remotehost,cmd)
bfile		*fpa[] ;
char		*environ[] ;
const char	remotehost[] ;
const char	cmd[] ;
{
	struct utsname	uts ;

	bfile	jobfile, *jfp = &jobfile ;

	pid_t	pid ;

	int	rs = SR_NOTOPEN ;
	int	i, len ;
	int	f_freecwd = FALSE ;
	int	f_ksh = FALSE ;
	int	klen ;
	int	vlen ;

	char	remotecmd[(MAXPATHLEN * 2) + 1] ;
	char	jobfname[MAXPATHLEN + 1] ;
	char	nodename[256 + 1], domainname[2048 + 1] ;
	char	valuebuf[MAXPATHLEN + 1] ;
	char	displaybuf[DISBUFLEN + 1] ;
	char	*cwd = NULL ;
	char	**ep ;
	char	*cp ;
	char	*cmd_shell = NULL ;
	char	*cmd_rcp, *cmd_rsh ;
	char	*vp, *nvp ;


#if	CF_DEBUGS
	debugprintf("bopenrcmde: entered host=%s cmd>\n%s\n",
	    remotehost,cmd) ;
#endif

	jobfname[0] = '\0' ;
	if ((remotehost == NULL) || (remotehost[0] == '\0'))
	    goto badhost ;

	if ((cmd == NULL) || (cmd[0] == '\0'))
	    goto badhost ;

/* OK so far, get some command names */

	cmd_rsh = "/bin/rsh" ;
	if (u_access(cmd_rsh,X_OK) != 0) {

	    cmd_rsh = "/bin/remsh" ;
	    if (u_access(cmd_rsh,X_OK) != 0) {

	        cmd_rsh = "/usr/ucb/rsh" ;
	        if (u_access(cmd_rsh,X_OK) != 0)
		    return SR_NOTSUP ;

	    }
	}

	cmd_rcp = "/bin/rcp" ;
	if (access(cmd_rcp,X_OK) != 0) {

	    cmd_rcp = "/usr/ucb/rcp" ;
	    if (u_access(cmd_rcp,X_OK) != 0)
		return SR_NOTSUP ;

	}

/* get our host (nodename) and domain name */

	getnodedomain(nodename,domainname) ;

/* get what SHELL we will be using */

	if ((cmd_shell = getenv("SHELL")) == NULL) {

	    f_ksh = TRUE ;
	    cmd_shell = "/bin/ksh" ;
	}

#if	CF_DEBUGS
	cmd_shell = "/home/dam/src/runadvice_1/ksh2" ;
	debugprintf("bopenrcmde: SHELL=%s\n",cmd_shell) ;
#endif

/* what SHELL to use on the remote side? */

	if ((! f_ksh) || (strcmp(strbasename(cmd_shell),"ksh") == 0))
	    f_ksh = TRUE ;

#if	CF_DEBUGS
	debugprintf("bopenrcmde: f_ksh=%d\n",f_ksh) ;
#endif

/* put together the remote command file */

	jobfname[0] = '\0' ;
	if ((rs = mkjobfile("/tmp",0740,jobfname)) < 0)
		goto badjobfile ;

	if ((rs = bopen(jfp,jobfname,"wct",0744)) < 0)
		goto badjobopen ;

#if	CF_DEBUGS
	debugprintf("bopenrcmde: jobfname=%s\n",jobfname) ;
#endif

/* write out our environment to the remote command */

	ep = environ ;
	for (i = 0 ; ep[i] != NULL ; i += 1) {

	    int	f_alloc ;


#if	CF_DEBUGS
	    debugprintf("bopenrcmde: top environment loop\n") ;
#endif

	    if ((ep[i][0] == '_') || (ep[i][0] < 32)) 
		continue ;

/* break it up into the two pieces */

	    f_alloc = FALSE ;
	    vp = "" ;
	    if ((cp = strchr(ep[i],'=')) != NULL) {

	        klen = cp - ep[i] ;	/* length of variable name */

#if	CF_DEBUGS
	        debugprintf("bopenrcmde: environment name=%W\n",ep[i],klen) ;
#endif

/* process the value part */

	        vp = ep[i] + klen + 1 ;

/* perform some preprocessing on specific variables */

	        if (strncmp(ep[i],"DISPLAY",klen) == 0) {

	            if (*vp == ':') {

#if	CF_DEBUGS
	                debugprintf("bopenrcmde: special DISPLAY\n") ;
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
	        debugprintf("bopenrcmde: about to call quote\n") ;
#endif

	        if ((rs = quotevalue(vp,valuebuf,MAXPATHLEN,&nvp)) < 0) {

#if	CF_DEBUGS
	            debugprintf("bopenrcmde: return bad alloc rs=%d\n",rs) ;
#endif

	            goto badalloc ;

	        }

#if	CF_DEBUGS
	        debugprintf("bopenrcmde: returned w/ rs=%d\n",rs) ;
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
	debugprintf("bopenrcmde: sent over the environment\n") ;
#endif

	bprintf(jfp,"RUNMODE=rcmd\nexport RUNMODE\n") ;

/* execute the remote command */

	if ((cwd = getenv("PWD")) == NULL) {

	    f_freecwd = TRUE ;
	    cwd = getcwd(NULL,0) ;

	}

	if (cwd != NULL)
	    bprintf(jfp,"cd %s 2> /dev/null\n",cwd) ;

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
	debugprintf("bopenrcmde: remotecmd>\n%s\n",remotecmd) ;
#endif

	pid = (pid_t) bopencmd(fpa,remotecmd) ;

#if	CF_DEBUGS
	debugprintf("bopenrcmde: spawned pid=%d\n",pid) ;
#endif

/* clean up */

	if ((f_freecwd) && (cwd != NULL))
	    uc_free(cwd) ;

	if (strcmp(nodename,remotehost) != 0)
	    u_unlink(jobfname) ;

ret0:
	return ((int) pid) ;

/* bad returns */
badhost2:
	if (f_freecwd) 
		uc_free(cwd) ;

badalloc:
badjobopen:
	u_unlink(jobfname) ;

badjobfile:
badret:

#if	CF_DEBUGS
	debugprintf("bopenrcmde: one of the error returns rs=%d\n",rs) ;
#endif

	goto ret0 ;

/* extra bad things come here */
badhost:
badcmd:
	rs = SR_INVALID ;
	goto badret ;
}
/* end subroutine (bopenrcmde) */


/* local subroutines */


#ifdef	COMMENT

/* fix up a DISPLAY environment variable content for domain name changes */
void fixdisplay(remotehost)
char	remotehost[] ;
{
	struct utsname		uts ;

	int	rs1 ;
	int	len, dlen ;
	int	size ;

	char	*hp ;
	char	*cp, *cp2 ;


/* fix up the environment variable DISPLAY, if found */

	if ((cp = getenv("DISPLAY")) == NULL) 
		return ;

/* is an adjustment needed? */

	if (cp[0] == ':') {

	    hp = remotehost ;
	    if ((remotehost == NULL) || (remotehost[0] == '\0')) {

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
char		vs[] ;
char		buf[] ;
int		buflen ;
char		**nvpp ;
{
	int	f_got = FALSE ;
	int	f_malloc = FALSE ;
	int	mlen, rlen = 0 ;
	int	curbuflen = buflen ;
	int	newbuflen ;
	int	blen ;
	int	rs ;

	char	*cp, *cp1 ;
	char	*curbuf = buf ;


#if	CF_DEBUGS
	debugprintf("bopenrcmde: entered value=%s\n",vs) ;
#endif

	cp = vs ;
	rlen = buflen ;
	blen = 0 ;
	while ((cp1 = strpbrk(cp,"\\\"")) != NULL) {

#if	CF_DEBUGS
	    debugprintf("bopenrcmde: got one\n") ;
#endif

	    f_got = TRUE ;
	    mlen = cp1 - cp ;
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
	    curbuf[blen++] = *cp1++ ;
	    rlen -= (mlen + 2) ;
	    cp = cp1 ;

#if	CF_DEBUGS
	    debugprintf("bopenrcmde: bottom of got one\n") ;
#endif

	} /* end while */

	if (f_got) {

#if	CF_DEBUGS
	    debugprintf("bopenrcmde: after got one\n") ;
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

	    memcpy(curbuf + blen,cp,mlen) ;

	    blen += mlen ;
	    rlen -= mlen ;
	    curbuf[blen] = '\0' ;

#if	CF_DEBUGS
	    debugprintf("bopenrcmde: bottom after got one\n") ;
#endif

	} /* end if (we got something) */

#if	CF_DEBUGS
	debugprintf("bopenrcmde: exiting f_got=%d\n",f_got) ;
#endif

	*nvpp = (f_got) ? curbuf : vs ;

#if	CF_DEBUGS
	debugprintf("bopenrcmde: exiting for real, blen=%d\n",blen) ;
#endif

	return blen ;

badalloc:
	if (f_malloc) 
		uc_free(curbuf) ;

	return BAD ;
}
/* end subroutine (quotevalue) */


static int newbuf(curbuf,curbuflen,f,nbpp)
int	curbuflen, f ;
char	*curbuf ;
char	**nbpp ;
{
	int	rs = SR_OK ;
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


/* compare host names */
static int hostequiv(h1,h2,localdomain)
const char	h1[], h2[] ;
const char	localdomain[] ;
{
	int	len1, len2 ;
	int	f_h1 = FALSE ;
	int	f_h2 = FALSE ;

	char	*cp, *cp1, *cp2 ;


	if ((cp1 = strchr(h1,'.')) != NULL) 
		f_h1 = TRUE ;

	if ((cp2 = strchr(h2,'.')) != NULL) 
		f_h2 = TRUE ;

	if (LEQUIV(f_h1,f_h2))
	    return (! strcasecmp(h1,h2)) ;

	if (f_h1) {

	    len1 = cp1 - h1 ;
	    len2 = strlen(h2) ;

	    if (len1 != len2) 
		return FALSE ;

	    cp1 += 1 ;
	    if (strcasecmp(cp1,localdomain) != 0) 
		return FALSE ;

	    return (strncasecmp(h1,h2,len1) == 0) ;

	}

	len1 = strlen(h1) ;

	len2 = cp2 - h2 ;
	if (len1 != len2) 
		return FALSE ;

	cp2 += 1 ;
	if (strcasecmp(cp2,localdomain) != 0) 
		return FALSE ;

	return (strncasecmp(h1,h2,len2) == 0) ;
}
/* end subroutine (hostequiv) */



