/* rex */

/* subroutine to open a FD to a remotely executing  command */


#define	CF_DEBUG	0
#define	CF_REXECL	1
#define	CF_RCMDU	1


/* revision history:

	- David A.D. Morano, 96/11/21

	This program was started by copying from the RSLOW program.


	- David A.D. Morano, 96/12/12

	I modified the program to take the username and password
	from a specified file (for better security).



*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Calling synopsis :

	int rex(rhost,auth,f,envfname,program,argv,envp,fd2p,mpp)
	char	rhost[] ;
	char	envfname[] ;
	char	program[] ;
	char	argv[], envp[] ;
	int	*fd2p ;
	struct rex_flags {
		uint	keepalive : 1 ;
		uint	changedir : 1 ;
	} *f ;
	struct rex_auth {
		char	*restrict ;
		char	*rusername ;
		char	*rpassword ;
		struct netrc	**machinev ;
	} *auth ;
	struct netrc	**mpp ;


	The subroutine either returns a FD for the remote command's
	standard input and standard output or it returns an error
	which is indicated by a negative valued return.

	If 'fd2p' is non-NULL, a secondary channel to the standard
	error of the remote command is created also.

	Depending on the arguments to the subroutine call, both
	the INET 'exec' or 'shell' services may be invoked to
	try and make a connection to the remote host.


**************************************************************************/


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

#include	"netfile.h"
#include	"rex.h"


/* local defines */

#define	BUFLEN		(8 * 1024)
#define	USERBUFLEN	(2 * 1024)
#define	CMDBUFLEN	8192
#define	DISPLAYLEN	(MAXHOSTNAMELEN + 50)


/* external subroutines */

extern int	getnodedomain() ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getchostname(), getehostname() ;
extern int	quoteshellarg(), qualdisplay() ;

extern char	*strshrink() ;
extern char	*strbasename() ;


/* forward subroutines */

static int	hostequiv() ;
static int	rex_rexec() ;
static int	rex_rcmd() ;
static int	strnncmp() ;


/* external variables */

extern int	errno ;


/* local variables */




int rex(rhost,auth,f,envfname,program,argv,envp,fd2p,mpp)
char	rhost[] ;
char	envfname[] ;
char	program[] ;
char	*argv[], *envp[] ;
int	*fd2p ;
struct rex_auth		*auth ;
struct rex_flags	*f ;
struct netrc		**mpp ;
{

	struct servent	*sp, se ;

	struct userinfo	u ;

	struct netrc	*mp ;

	int	i, j ;
	int	srs, rs, len, l ;
	int	rfd ;
	int	childstat ;
	int	port = -1 ;

	char	buf[BUFLEN + 1], *bp ;
	char	userbuf[USERBUFLEN + 1] ;
	char	*username = NULL ;
	char	*password = NULL ;
	char	*authorization = "any" ;
	char	*cp, *cp1, *cp2 ;
	char	cmdbuf[CMDBUFLEN + 1] ;
	char	displaybuf[DISPLAYLEN + 1] ;
	char	hostname[MAXHOSTNAMELEN + 1] ;
	char	wormfname[MAXPATHLEN + 1] ;
	char	*args[2] ;


#if	CF_DEBUG
	debugprintf("rex: entered\n") ;
#endif

/* initialize some other common user stuff */

	wormfname[0] = '\0' ;

	if ((rs = userinfo(&u,userbuf,USERBUFLEN,NULL)) < 0)
	    if (u.domainname == NULL) u.domainname = "" ;

#if	CF_DEBUG
	debugprintf("rex: got user information\n") ;
#endif


	if ((rhost == NULL) || (rhost[0] == '\0'))
	    goto badhost ;

	if ((program == NULL) || (program[0] == '\0'))
	    goto badprog ;

	if (argv == NULL) {

	    args[0] = strbasename(program) ;

	    args[1] = NULL ;
	    argv = args ;

	}

/* laod up some authorization information that we have */

	if (auth != NULL) {

	    if (auth->username != NULL) username = auth->username ;

	    if (auth->password != NULL) password = auth->password ;

	    if (auth->restrict != NULL) authorization = auth->restrict ;

	}


	if ((username == NULL) || (username[0] == '\0'))
	    username = u.username ;


/* process the 'rhost' to see if there is a port (or service) also */

	if ((cp = strchr(rhost,':')) != NULL) {

	    strwcpy(hostname,rhost,cp - rhost) ;

	    cp += 1 ;
	    if (cfdeci(cp,-1,&port) < 0) {

	        port = -1 ;

#if	SYSV
	        sp = getservbyname_r(cp, "tcp",
	            &se,cmdbuf,CMDBUFLEN) ;
#else
	        sp = getservbyname(cp, "tcp") ;
#endif

	        if (sp != NULL) {

	            port = (int) ntohs(sp->s_port) ;

#if	CF_DEBUG
	            debugprintf("rex: service specified port=%d\n",
	                port) ;
#endif

	        }

	    } /* end if (bad decimal conversion) */

#if	CF_DEBUG
	    debugprintf("rex: specified port=%d\n",
	        port) ;
#endif

	} else
	    strcpy(hostname,rhost) ;


/* get the INET service port for the REXEC service */

	if (port < 0) {

#if	SYSV
	    sp = getservbyname_r("exec", "tcp",
	        &se,cmdbuf,CMDBUFLEN) ;
#else
	    sp = getservbyname("exec", "tcp") ;
#endif

	    if (sp == NULL) {

#if	SYSV
	        sp = getservbyname_r("rexec", "tcp",
	            &se,cmdbuf,CMDBUFLEN) ;
#else
	        sp = getservbyname("rexec", "tcp") ;
#endif

	    }

	    if (sp != NULL)
	        port = (int) ntohs(sp->s_port) ;

	    else
	        port = REX_DEFEXECSERVICE ;

	} /* end if (default port) */


/* create the remote command */

#if	CF_DEBUG
	debugprintf("rex: creating remote command\n") ;
#endif

	len = sprintf(cmdbuf,"%s",program) ;

	if ((argv != NULL) && (argv[0] != NULL)) {

	    for (i = 1 ; argv[i] != NULL ; i += 1) {

#if	CF_DEBUG
	        debugprintf("rex: arg%d> %s\n",i,argv[i]) ;
#endif

	        l = quoteshellarg(argv[i],-1,buf,BUFLEN,&bp) ;

	        if ((l < 0) || (l > (CMDBUFLEN - len)))
	            goto badtoomuch ;

#if	CF_DEBUG
	        debugprintf("rex: sharg%d> %W\n",i,bp,l) ;
#endif

	        cmdbuf[len++] = ' ' ;
	        strwcpy(cmdbuf + len,bp,l) ;

	        len += l ;

	    } /* end for */

	} /* end if ('argv' not NULL) */

#if	CF_DEBUG
	debugprintf("rex: cmd=\"%s\"\n",cmdbuf) ;
#endif

/* was some environment specified by the caller ? */

	if (envp != NULL) {

	    bfile		jfile, *jfp = &jfile ;

	    int	klen ;

	    char	*kp, *vp ;


	    if ((rs = mkjobfile("/tmp",0600,wormfname)) < 0)
	        goto badjobmk ;

	    if ((rs = bopen(jfp,wormfname,"wct",0600)) < 0)
	        goto badjobopen ;

	    bprintf(jfp," # <-- force CSH to use Bourne shell\n") ;

	    bprintf(jfp,"# REX(3)\n") ;

	    bprintf(jfp,": ${LOGNAME:=${USER}}\nexport LOGNAME\n") ;

	    bprintf(jfp,": ${PWD:=${HOME}}\nexport PWD\n") ;

	    if ((envfname != NULL) && (envfname[0] != '\0'))
	        bprintf(jfp,"if [ -r %s ] ; then . %s ; fi\n",
	            envfname) ;

	    for (i = 0 ; envp[i] != NULL ; i += 1) {

	        if ((cp = strchr(envp[i],'=')) == NULL) continue ;

	        if (envp[i][0] == '_') continue ;

	        kp = envp[i] ;
	        klen = cp - envp[i] ;
	        if ((strnncmp(kp,klen,"HOME",4) == 0) ||
	            (strnncmp(kp,klen,"PWD",3) == 0) ||
	            (strnncmp(kp,klen,"LOGNAME",7) == 0) ||
	            (strnncmp(kp,klen,"HZ",2) == 0) ||
	            (strnncmp(kp,klen,"LOCALDOMAIN",11) == 0) ||
	            (strnncmp(kp,klen,"TERM",4) == 0)) continue ;

	        vp = kp + klen + 1 ;
	        if (strnncmp(kp,klen,"DISPLAY",7) == 0) {

	            qualdisplay(vp,u.nodename,u.domainname,
	                displaybuf,DISPLAYLEN) ;

	            vp = displaybuf ;

	        } /* end if (qualifying the X window display specification) */

	        if ((l = quoteshellarg(vp,-1,buf,BUFLEN,&vp)) < 0)
	            continue ;

	        bprintf(jfp,"%W=%W\nexport %W\n",kp,klen,vp,l,kp,klen) ;

	    } /* end for (looping through the environment) */

/* send over the fixed stuff */


	    bprintf(jfp,"RUNMODE=rcmd\nexport RUNMODE\n",username) ;

	    bprintf(jfp,"ORIGHOSTNAME=%s\nexport ORIGHOSTNAME\n",
	        hostname) ;

	    bprintf(jfp,"ORIGLOGNAME=%s\nexport ORIGLOGNAME\n",
	        u.username) ;


/* do the standard environment variables */

	    if (f->changedir) {

	        char	cwd[MAXPATHLEN + 1] ;


	        if (getcwd(cwd,MAXPATHLEN) != NULL)
	            bprintf(jfp,"cd %s 2> /dev/null\n",cwd) ;

	    }

	    if (strncmp(cmdbuf,"exec",4) != 0)
	        bprintf(jfp,"exec ") ;

	bwrite(jfp,cmdbuf,strlen(cmdbuf)) ; bprintf(jfp,"\n") ;

	    bprintf(jfp,"exit 1\n") ;

	    bclose(jfp) ;

	} /* end if (environment specified) */

/* now we want to make sure that our remote hostname is INET translatable */

	if (inet_addr(hostname) == 0xFFFFFFFF) {

	    if (getehostname(hostname,buf) < 0) goto badunreach ;

	    if (strcmp(hostname,buf) != 0)
	        strcpy(hostname,buf) ;

	}

/* try the supplied password that we have been given */

	if (password != NULL) {

#if	CF_DEBUG
	    debugprintf("rex: executing REXEC on supplied password\n") ;
#endif

#if	CF_DEBUG
	    debugprintf("rex: supplied m=\"%s\" u=\"%s\" p=\"%s\"\n",
	        hostname,username,password) ;
#endif

	    rs = rex_rexec(hostname,port,username,
	        password,wormfname,cmdbuf,fd2p) ;

#if	CF_DEBUG
	    debugprintf("rex: back from REXECL w/ rs=%d\n",
	        rs) ;
#endif

	    rfd = rs ;
	    if (rs >= 0) goto goodsupply ;

	} /* end if (tried supplied password) */

	if (strncmp(authorization,"supplied",1) == 0) goto badrexec ;


/* process any NETRC files that we can find */

#if	CF_DEBUG
	debugprintf("rex: about to process the NETRC files\n") ;
#endif


/* try to find a matching NETRC entry for the host/user pair that we have */

	if ((auth != NULL) && (auth->machinev != NULL) &&
	    (strncmp(authorization,"r",1) != 0)) {

#if	CF_DEBUG
	    debugprintf("rex: m=%s u=%s\n",hostname,username) ;
#endif

	    rs = -1 ;
	    for (i = 0 ; (mp = auth->machinev[i]) != NULL ; i += 1) {

#if	CF_DEBUG
	        debugprintf("rex: entry i=%d\n",
	            i) ;

	        if (mp->machine != NULL)
	            debugprintf("rex: m=\"%s\"\n",
	                mp->machine) ;

	        if (mp->login != NULL)
	            debugprintf("rex: u=\"%s\"\n",
	                mp->login) ;

	        if (mp->password != NULL)
	            debugprintf("rex: p=\"%s\"\n",
	                mp->password) ;
#endif

	        if ((mp->machine == NULL) ||
	            (mp->login == NULL) ||
	            (mp->password == NULL)) continue ;

#if	CF_DEBUG
	        debugprintf("rex: looking at entry m=\"%s\"\n",
	            mp->machine) ;
#endif

	        if ((hostequiv(hostname,mp->machine,u.domainname)) &&
	            (strcmp(mp->login,username) == 0)) {

#if	CF_DEBUG
	            debugprintf("rex: trying entry m=\"%s\" u=\"%s\"\n",
	                mp->machine,mp->login) ;
#endif

	            rs = rex_rexec(hostname,port,username,
	                mp->password,wormfname,cmdbuf,fd2p) ;

	            rfd = rs ;
	            if (rs >= 0) break ;

	        } /* end if */

	    } /* end for */

	    if (rs >= 0) goto goodrexec ;

#if	CF_DEBUG
	    debugprintf("rex: no matching NETRC entries\n") ;
#endif

	} /* end if (we have authorization machine entries) */

	if (strncmp(authorization,"password",1) == 0) goto badrexec ;


#if	CF_RCMDU

/* try RCMDU with the supplied username */

	if ((f == NULL) || ((f != NULL) && (! f->keepalive))) {

#if	CF_DEBUG
	    debugprintf("rex: executing RCMDU u=%s\n",username) ;
#endif

	    rs = rex_rcmd(hostname,username,
	        wormfname,cmdbuf,fd2p) ;

	    rfd = rs ;
	    if (rs >= 0) {

#if	CF_DEBUG
	        debugprintf("rex: connected w/ RCMDU\n") ;
#endif

	        goto goodrcmdu ;
	    }

	    if ((rs == SR_HOSTDOWN) || (rs == SR_HOSTUNREACH))
	        goto badhostdown ;

	} /* end if */

#endif

	if (strncmp(authorization,"user",1) == 0) goto badrexec ;


/* try no password on the specified username */

#if	CF_DEBUG
	debugprintf("rex: executing REXEC with blank password\n") ;
#endif

	rs = rex_rexec(hostname,port,username,
	    "",wormfname,cmdbuf,fd2p) ;

	rfd = rs ;
	if (rs >= 0) goto goodrexec ;

/* we couldn't get through with a NULL password */


/* try to find a matching NETRC entry for the host only */

	if ((auth != NULL) && (auth->machinev != NULL) &&
	    (strncmp(authorization,"r",1) != 0)) {

#if	CF_DEBUG
	    debugprintf("rex: autologin m=%s\n",hostname) ;
#endif

	    rs = -1 ;
	    for (i = 0 ; (mp = auth->machinev[i]) != NULL ; i += 1) {

#if	CF_DEBUG
	        debugprintf("rex: entry i=%d\n",
	            i) ;

	        if (mp->machine != NULL)
	            debugprintf("rex: m=\"%s\"\n",
	                mp->machine) ;

	        if (mp->login != NULL)
	            debugprintf("rex: u=\"%s\"\n",
	                mp->login) ;

	        if (mp->password != NULL)
	            debugprintf("rex: p=\"%s\"\n",
	                mp->password) ;
#endif

	        if ((mp->machine == NULL) ||
	            (mp->login == NULL)) continue ;

#if	CF_DEBUG
	        debugprintf("rex: looking at entry m=\"%s\"\n",
	            mp->machine) ;
#endif

	        if (hostequiv(hostname,mp->machine,u.domainname) &&
	            (strcasecmp(mp->login,username) != 0)) {

#if	CF_DEBUG
	            debugprintf("rex: trying entry m=\"%s\" u=\"%s\"\n",
	                mp->machine,mp->login) ;
#endif

	            rs = rex_rexec(hostname,port,mp->login,
	                mp->password,wormfname,cmdbuf,fd2p) ;

	            rfd = rs ;
	            if (rs >= 0) break ;

	        }

	    } /* end for */

	    if (rs >= 0) goto goodrexec ;

#if	CF_DEBUG
	    debugprintf("rex: couldn't find a NETRC entry \n") ;
#endif

	} /* end if (we had authorization NETRC information) */

/* done with this try */


#if	CF_RCMDU

/* finally we try to connect using RCMDU with our own username */

	if (((f == NULL) || ((f != NULL) && (! f->keepalive))) && 
	    (strcmp(username,u.username) != 0)) {

#if	CF_DEBUG
	    debugprintf("rex: use our own username w/ RCMDU\n") ;
#endif

	    rs = rex_rcmd(hostname,u.username,
	        wormfname,cmdbuf,fd2p) ;

	    rfd = rs ;
	    if (rs >= 0) goto goodrcmdu ;

	    if ((rs == SR_HOSTDOWN) || (rs == SR_HOSTUNREACH))
	        goto badhostdown ;

	} /* end if */

#endif


/* we failed all attempts */

#if	CF_DEBUG
	debugprintf("rex: failed all authorization attempts\n") ;
#endif

	goto badrexec ;



/* we got in ! */
goodsupply:
goodrcmdu:
	if (mpp != NULL)
	    *mpp = NULL ;

	goto done ;

goodrexec:
	if (mpp != NULL)
	    *mpp = mp ;

done:

#if	CF_DEBUG
	debugprintf("rex: finishing up\n") ;
#endif

	if (wormfname[0] != '\0') unlink(wormfname) ;

	return rfd ;

/* bad returns come here */
badret:
	if (wormfname[0] != '\0') unlink(wormfname) ;

	return srs ;

badhost:
	srs = SR_INVAL ;
	goto badret ;

badprog:
	srs = SR_INVAL ;
	goto badret ;

badjobmk:
	srs = SR_2BIG ;
	goto badret ;

badjobopen:
	srs = SR_2BIG ;
	goto badret ;

badev:
	srs = SR_2BIG ;
	goto badret ;

badtoomuch:
	srs = SR_2BIG ;
	goto badret ;

badhostdown:
	srs = rs ;
	goto badret ;

badunreach:
	srs = SR_HOSTUNREACH ;
	goto badret ;

badrexec:
	srs = SR_ACCES ;
	goto badret ;

}
/* end subroutine (main) */


/* hostequiv */

/* rough equivalent host check */



/* revision history:

	- David A.D. Morano, 96/11/21
	This program was started by copying from the RSLOW program.

	- David A.D. Morano, 96/12/12
	I modified the program to take the username and password
	from a specified file (for better security).


*/



/**************************************************************************

	Call as :



**************************************************************************/



/* compare host names */
static int hostequiv(h1,h2,localdomain)
char	h1[], h2[] ;
char	localdomain[] ;
{
	int	f_h1 = FALSE, f_h2 = FALSE ;
	int	len1, len2 ;

	char	*cp, *cp1, *cp2 ;


	if ((cp1 = strchr(h1,'.')) != NULL) f_h1 = TRUE ;

	if ((cp2 = strchr(h2,'.')) != NULL) f_h2 = TRUE ;

	if (LEQUIV(f_h1,f_h2))
	    return (! strcasecmp(h1,h2)) ;

	if (f_h1) {

	    len1 = cp1 - h1 ;
	    len2 = strlen(h2) ;

	    if (len1 != len2) return FALSE ;

	    cp1 += 1 ;
	    if (strcasecmp(cp1,localdomain) != 0) return FALSE ;

	    return (strncasecmp(h1,h2,len1) == 0) ;

	}

	len1 = strlen(h1) ;

	len2 = cp2 - h2 ;
	if (len1 != len2) return FALSE ;

	cp2 += 1 ;
	if (strcasecmp(cp2,localdomain) != 0) return FALSE ;

	return (strncasecmp(h1,h2,len2) == 0) ;
}
/* end subroutine (hostequiv) */



static int rex_rexec(hostname,port,username,password,worm,cmd,fd2p)
char	hostname[] ;
int	port ;
char	username[], password[] ;
char	worm[] ;
char	cmd[] ;
int	*fd2p ;
{
	int	rs, wfd, rfd ;
	int	len ;

	char	buf[BUFLEN + 1] ;
	char	cmdbuf[(MAXPATHLEN * 2) + 1] ;
	char	ahostname[MAXHOSTNAMELEN + 1] ;
	char	*ahost ;


	if (worm != NULL) {

	    if ((wfd = open(worm,O_RDONLY,0666)) < 0)
	        return (- errno) ;

	    sprintf(cmdbuf,"/bin/cat > %s",worm) ;

	    ahost = ahostname ;
	    strcpy(ahostname,hostname) ;

#if	CF_DEBUG
	    debugprintf("rex: calling REXECL\n") ;
#endif

#if	CF_REXECL
	    rfd = rexecl(&ahost,(unsigned short) port,username,
	        password,cmdbuf,NULL) ;
#else
	    if (password == NULL) password = "" ;

	    rfd = rexec(&ahost,(unsigned short) port,username,
	        password,cmdbuf,NULL) ;
#endif

	    if (rfd < 0) {

	        close(wfd) ;

	        return rfd ;
	    }

	    rs = 0 ;
	    while ((len = read(wfd,buf,BUFLEN)) > 0) {

	        if ((rs = writen(rfd,buf,len)) < 0) break ;

	    }

	    if (len < 0) len = (- errno) ;

	    shutdown(rfd,2) ;

	    close(wfd) ;

	    close(rfd) ;

	    if (len < 0) return len ;

	    if (rs < 0) return rs ;

	    cmd = cmdbuf ;
	    sprintf(cmdbuf,"/bin/sh %s",worm) ;

	} /* end if (sending the worm over) */

	ahost = ahostname ;
	strcpy(ahostname,hostname) ;

#if	CF_DEBUG
	debugprintf("rex: calling REXECL\n") ;
#endif

#if	CF_REXECL
	rs = rexecl(&ahost,(unsigned short) port,username,
	    password,cmd,fd2p) ;
#else
	if (password == NULL) password = "" ;

	rs = rexec(&ahost,(unsigned short) port,username,
	    password,cmd,fd2p) ;
#endif

	return rs ;
}
/* end subroutine (rex_rexec) */



static int rex_rcmd(hostname,username,worm,cmd,fd2p)
char	hostname[] ;
char	username[] ;
char	worm[] ;
char	cmd[] ;
int	*fd2p ;
{
	int	rs, rfd, wfd ;
	int	len ;

	char	buf[BUFLEN + 1] ;
	char	cmdbuf[(MAXPATHLEN * 2) + 1] ;


	if (worm != NULL) {

	    if ((wfd = open(worm,O_RDONLY,0666)) < 0)
	        return (- errno) ;

	    sprintf(cmdbuf,"/bin/cat > %s",worm) ;

#if	CF_DEBUG
	    debugprintf("rex: calling RCMDU\n") ;
#endif

	    rfd = rcmdu(hostname,username,
	        cmdbuf,NULL) ;

	    if (rfd < 0) {

	        close(wfd) ;

	        return rfd ;
	    }

	    rs = 0 ;
	    while ((len = read(wfd,buf,BUFLEN)) > 0) {

	        if ((rs = writen(rfd,buf,len)) < 0) break ;

	    }

	    if (len < 0) len = (- errno) ;

	    shutdown(rfd,2) ;

	    close(wfd) ;

	    close(rfd) ;

	    if (len < 0) return len ;

	    if (rs < 0) return rs ;

	    cmd = cmdbuf ;
	    sprintf(cmdbuf,"/bin/sh %s",worm) ;

	} /* end if (sending the worm over) */

#if	CF_DEBUG
	debugprintf("rex: calling RCMDU\n") ;
#endif

	rs = rcmdu(hostname,username,
	    cmd,fd2p) ;

	return rs ;
}
/* end subroutine (rex_rcmd) */


static int strnncmp(s1,n1,s2,n2)
char	s1[], s2[] ;
int	n1, n2 ;
{
	int	n, rs ;


	if (n1 < 0) n1 = strlen(s1) ;

	if (n2 < 0) n2 = strlen(s2) ;

	n = MIN(n1,n2) ;
	if ((rs = strncmp(s1,s2,n)) != 0) return rs ;

	return (n1 - n2) ;
}



