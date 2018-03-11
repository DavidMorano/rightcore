/* rex */

/* subroutine to open a FD to a remotely executing command */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_REXECL	1		/* use 'rxecl(3dam)' */
#define	CF_RCMDU	1		/* use 'rcmdu(3dam)' */


/* revision history:

	= 1996-11-21, David A­D­ Morano
	This program was started by copying from the RSLOW program.

	= 1996-12-12, David A­D­ Morano
        I modified the program to take the username and password from a
        specified file (for better security).

*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int rex(rhost,auth,f,program,argv,fd2p,mpp)
	char	rhost[] ;
	char	program[] ;
	char	argv[] ;
	int	*fd2p ;
	struct rex_flags {
		uint	keepalive:1 ;
	} *f ;
	struct rex_auth {
		char	*restrict ;
		char	*rusername ;
		char	*rpassword ;
		NETFILE_ENT	**machinev ;
	} *auth ;
	NETFILE_ENT		**mpp ;


	The subroutine either returns a FD for the remote command's
	standard input and standard output or it returns an error
	which is indicated by a negative valued return.

	If 'fd2p' is non-NULL, a secondary channel to the standard
	error of the remote command is created also.

	Depending on the arguments to the subroutine call, both
	the INET 'exec' or 'shell' services may be invoked to
	try and make a connection to the remote host.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/param.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<strings.h>		/* |strncasecmp(3c)| */
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<netfile.h>
#include	<localmisc.h>

#include	"rex.h"


/* local defines */

#define	BUFLEN		(8 * 1024)
#define	USERBUFLEN	(2 * 1024)
#define	CMDBUFLEN	8192

#ifndef	LEQUIV
#define	LEQUIV(a,b)	(((a) && (b)) || ((! (a)) && (! (b))))
#endif

#define	CF_USEREENTRANT	defined(POSIX) && defined(SOLARIS)

#ifndef	INETADDRBAD
#define	INETADDRBAD	((unsigned int) (~ 0))
#endif


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern int	strncasecmp(const char *,const char *,int) ;
#endif

extern int	sfbasename(const char *,int,const char **) ;
extern int	getnodedomain(char *,char *) ;
extern int	getchostname() ;
extern int	getehostname() ;
extern int	mkquoted(char *,int,const char *,int) ;


/* forward subroutines */

static int	hostequiv() ;


/* external variables */


/* local variables */


/* exported subroutines */


int rex(rhost,auth,f,program,argv,fd2p,mpp)
const char	rhost[] ;
REX_AUTH	*auth ;
REX_FL		*f ;
const char	program[] ;
const char	**argv ;
int		*fd2p ;
NETFILE_ENT	**mpp ;
{
	struct servent	*sp, se ;
	USERINFO	u ;
	NETFILE_ENT	*mp ;
	int		rs = SR_OK ;
	int		srs ;
	int		i, j ;
	int		len, l ;
	int		childstat ;
	int		rfd = -1 ;
	int		port = -1 ;
	const char	*args[2] ;
	const char	*username = NULL ;
	char		userbuf[USERBUFLEN + 1] ;
	char		buf[BUFLEN + 1], *bp ;
	char		cmdbuf[CMDBUFLEN + 1] ;
	char		hostname[MAXHOSTNAMELEN + 1] ;
	char		ahostname[MAXHOSTNAMELEN + 1] ;
	const char	*password = NULL ;
	const char	*authorization = "any" ;
	const char	*cp, *cp1, *cp2 ;
	char		*ahost = ahostname ;

#if	CF_DEBUGS
	debugprintf("rex: ent\n") ;
#endif

/* initialize some other common user stuff */

	if ((rs = userinfo(&u,userbuf,USERBUFLEN,NULL)) < 0) {
	    if (u.domainname == NULL) u.domainname = "" ;
	}

#if	CF_DEBUGS
	debugprintf("rex: goto user information\n") ;
#endif

	if ((rhost == NULL) || (rhost[0] == '\0'))
	    goto badhost ;

	if ((program == NULL) || (program[0] == '\0'))
	    goto badprog ;

	if (argv == NULL) {
	    const char	*ccp ;

	    sfbasename(program,-1,&ccp) ;

	    args[0] = ccp ;
	    args[1] = NULL ;
	    argv = args ;

	} /* end if */

/* laod up some authorization information that we have */

	if (auth != NULL) {

	    if (auth->username != NULL) 
		username = auth->username ;

	    if (auth->password != NULL) 
		password = auth->password ;

	    if (auth->restrict != NULL) 
		authorization = auth->restrict ;

	} /* end if */

	if ((username == NULL) || (username[0] == '\0'))
	    username = u.username ;

/* process the 'rhost' to see if there is a port (or service) also */

	if ((cp = strchr(rhost,':')) != NULL) {

	    strwcpy(hostname,rhost,cp - rhost) ;

	    cp += 1 ;
	    if (cfdec(cp,-1,&port) < 0) {

	        port = -1 ;

#if	CF_USEREENTRANT
	        sp = getservbyname_r(cp, "tcp",
	            &se,cmdbuf,CMDBUFLEN) ;
#else
	        sp = getservbyname(cp, "tcp") ;
#endif

	        if (sp != NULL) {

	            port = (int) ntohs(sp->s_port) ;

#if	CF_DEBUGS
	            debugprintf("rex: service specified port=%d\n",
	                port) ;
#endif

	        }

	    } /* end if (bad decimal conversion) */

#if	CF_DEBUGS
	    debugprintf("rex: specified port=%d\n",
	        port) ;
#endif

	} else
	    strcpy(hostname,rhost) ;

/* get the INET service port for the REXEC service */

	if (port < 0) {

#if	CF_USEREENTRANT
	    sp = getservbyname_r("exec", "tcp",
	        &se,cmdbuf,CMDBUFLEN) ;
#else
	    sp = getservbyname("exec", "tcp") ;
#endif

	    if (sp == NULL) {

#if	CF_USEREENTRANT
	        sp = getservbyname_r("rexec", "tcp",
	            &se,cmdbuf,CMDBUFLEN) ;
#else
	        sp = getservbyname("rexec", "tcp") ;
#endif

	    }

	    if (sp != NULL) {
	        port = (int) ntohs(sp->s_port) ;
	    } else {
	        port = REX_DEFEXECSERVICE ;
	    }

	} /* end if (default port) */

/* create the remote command */

#if	CF_DEBUGS
	debugprintf("rex: creating remote command\n") ;
#endif

	len = bufprintf(cmdbuf,CMDBUFLEN,"%s",program) ;

	if ((argv != NULL) && (argv[0] != NULL)) {

	    for (i = 1 ; argv[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("rex: arg%d> %s\n",i,argv[i]) ;
#endif

		bp = buf ;
	        l = mkquoted(buf,BUFLEN,argv[i],-1) ;

	        if ((l < 0) || (l > (CMDBUFLEN - len)))
	            goto badtoomuch ;

#if	CF_DEBUGS
	        debugprintf("rex: sharg%d> %W\n",i,bp,l) ;
#endif

	        cmdbuf[len++] = ' ' ;
	        strwcpy(cmdbuf + len,bp,l) ;

	        len += l ;

	    } /* end for */

	} /* end if ('argv' not NULL) */

#if	CF_DEBUGS
	debugprintf("rex: cmd=\"%s\"\n",cmdbuf) ;
#endif

/* now we want to make sure that our remote hostname is INET translatable */

	{
	    in_addr_t	addr ;

	    addr = inet_addr(hostname) ;
	    if (addr == INETADDRBAD) {

	        if (getehostname(hostname,buf) < 0) 
		    goto badunreach ;

	        if (strcmp(hostname,buf) != 0)
	            strcpy(hostname,buf) ;

	    } /* end if */
	} /* end block */

/* try the supplied password that we have been given */

	if (password != NULL) {

#if	CF_DEBUGS
	    debugprintf("rex: executing REXEC on supplied password\n") ;
#endif

#if	CF_DEBUGS
	    debugprintf("rex: supplied m=\"%s\" u=\"%s\" p=\"%s\"\n",
	        hostname,username,password) ;
#endif

	    ahost = ahostname ;
	    strcpy(ahostname,hostname) ;

#if	CF_DEBUGS
	    debugprintf("rex: calling REXECL\n") ;
#endif

#if	CF_REXECL
	    rs = rexecl(&ahost,port,username,
	        password, cmdbuf,fd2p) ;
#else
	    if (password == NULL) password = "" ;

	    rs = rexec(&ahost,port,username,
	        password, cmdbuf,fd2p) ;
#endif /* R_REXECL */

#if	CF_DEBUGS
	    debugprintf("rex: back from REXECL w/ rs=%d\n",
	        rs) ;
#endif

	    rfd = rs ;
	    if (rs >= 0) goto goodsupply ;

	} /* end if (tried supplied password) */

	if (strncmp(authorization,"supplied",1) == 0) 
		goto badrexec ;

/* process any NETRC files that we can find */

#if	CF_DEBUGS
	debugprintf("rex: about to process the NETRC files\n") ;
#endif

/* try to find a matching NETRC entry for the host/user pair that we have */

	if ((auth != NULL) && (auth->machinev != NULL) &&
	    (strncmp(authorization,"r",1) != 0)) {

#if	CF_DEBUGS
	    debugprintf("rex: m=%s u=%s\n",hostname,username) ;
#endif

	    rs = -1 ;
	    for (i = 0 ; (mp = auth->machinev[i]) != NULL ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("rex: entry i=%d\n", i) ;
	        if (mp->machine != NULL)
	            debugprintf("rex: m=\"%s\"\n",
	                mp->machine) ;
	        if (mp->login != NULL)
	            debugprintf("rex: u=\"%s\"\n",
	                mp->login) ;
	        if (mp->password != NULL)
	            debugprintf("rex: p=\"%s\"\n",
	                mp->password) ;
#endif /* CF_DEBUGS */

	        if ((mp->machine == NULL) ||
	            (mp->login == NULL) ||
	            (mp->password == NULL)) continue ;

#if	CF_DEBUGS
	        debugprintf("rex: looking at entry m=\"%s\"\n",
	            mp->machine) ;
#endif

	        if ((hostequiv(hostname,mp->machine,u.domainname)) &&
	            (strcmp(mp->login,username) == 0)) {

#if	CF_DEBUGS
	            debugprintf("rex: trying entry m=\"%s\" u=\"%s\"\n",
	                mp->machine,mp->login) ;
#endif

	            ahost = ahostname ;
	            strcpy(ahostname,hostname) ;

#if	CF_REXECL
	            rs = rexecl(&ahost,port,username,
	                mp->password, cmdbuf,fd2p) ;
#else
	            password = mp->password ;
	            if (password == NULL) password = "" ;

	            rs = rexec(&ahost,port,username,
	                password, cmdbuf,fd2p) ;
#endif

	            rfd = rs ;
	            if (rs >= 0) break ;

	        } /* end if */

	    } /* end for */

	    if (rs >= 0) goto goodrexec ;

#if	CF_DEBUGS
	    debugprintf("rex: no matching NETRC entries\n") ;
#endif

	} /* end if (we have authorization machine entries) */

	if (strncmp(authorization,"password",1) == 0) 
		goto badrexec ;

#if	CF_RCMDU

/* try RCMDU with the supplied username */

	if ((f == NULL) || ((f != NULL) && (! f->keepalive))) {

#if	CF_DEBUGS
	    debugprintf("rex: executing RCMDU u=%s\n",username) ;
#endif

	    rs = rcmdu(hostname,username, cmdbuf,fd2p) ;
	    rfd = rs ;
	    if (rs >= 0) {

#if	CF_DEBUGS
	        debugprintf("rex: connected w/ RCMDU\n") ;
#endif

	        goto goodrcmdu ;
	    }

	    if ((rs == SR_HOSTDOWN) || (rs == SR_HOSTUNREACH))
	        goto hostdown ;

	} /* end if */

#endif

	if (strncmp(authorization,"user",1) == 0) 
		goto badrexec ;

/* try no password on the specified username */

#if	CF_DEBUGS
	debugprintf("rex: executing REXEC with blank password\n") ;
#endif

	ahost = ahostname ;
	strcpy(ahostname,hostname) ;

#if	CF_REXECL
	rs = rexecl(&ahost,port,username,
	    "", cmdbuf,fd2p) ;
#else
	rs = rexec(&ahost,port,username,
	    "", cmdbuf,fd2p) ;
#endif

	rfd = rs ;
	if (rs >= 0) 
		goto goodrexec ;

/* we couldn't get through with a NULL password */


/* try to find a matching NETRC entry for the host only */

	if ((auth != NULL) && (auth->machinev != NULL) &&
	    (strncmp(authorization,"r",1) != 0)) {

#if	CF_DEBUGS
	    debugprintf("rex: autologin m=%s\n",hostname) ;
#endif

	    rs = -1 ;
	    for (i = 0 ; (mp = auth->machinev[i]) != NULL ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("rex: entry i=%d\n", i) ;
	        if (mp->machine != NULL)
	            debugprintf("rex: m=\"%s\"\n", mp->machine) ;
	        if (mp->login != NULL)
	            debugprintf("rex: u=\"%s\"\n", mp->login) ;
	        if (mp->password != NULL)
	            debugprintf("rex: p=\"%s\"\n", mp->password) ;
#endif /* F_DDEBUG */

	        if ((mp->machine == NULL) ||
	            (mp->login == NULL)) continue ;

#if	CF_DEBUGS
	        debugprintf("rex: looking at entry m=\"%s\"\n",
	            mp->machine) ;
#endif

	        if (hostequiv(hostname,mp->machine,u.domainname) &&
	            (strcasecmp(mp->login,username) != 0)) {

#if	CF_DEBUGS
	            debugprintf("rex: trying entry m=\"%s\" u=\"%s\"\n",
	                mp->machine,mp->login) ;
#endif

	            ahost = ahostname ;
	            strcpy(ahostname,hostname) ;

#if	CF_REXECL
	            rs = rexecl(&ahost,port,mp->login,
	                mp->password, cmdbuf,fd2p) ;
#else
	            password = mp->password ;
	            if (password == NULL) password = "" ;

	            rs = rexec(&ahost,port,mp->login,
	                password, cmdbuf,fd2p) ;
#endif /* CF_REXECL */

	            rfd = rs ;
	            if (rs >= 0) break ;

	        }

	    } /* end for */

	    if (rs >= 0) goto goodrexec ;

#if	CF_DEBUGS
	    debugprintf("rex: couldn't find a NETRC entry \n") ;
#endif

	} /* end if (we had authorization NETRC information) */

/* done with this try */

#if	CF_RCMDU

/* finally we try to connect using RCMDU with our own username */

	if (((f == NULL) || ((f != NULL) && (! f->keepalive))) && 
	    (strcmp(username,u.username) != 0)) {

#if	CF_DEBUGS
	    debugprintf("rex: use our own username w/ RCMDU\n") ;
#endif

	    rs = rcmdu(hostname,u.username,
	        cmdbuf,fd2p) ;

	    rfd = rs ;
	    if (rs >= 0) goto goodrcmdu ;

	    if ((rs == SR_HOSTDOWN) || (rs == SR_HOSTUNREACH))
	        goto hostdown ;

	} /* end if */

#endif /* CF_RCMDU */

/* we failed all attempts */

#if	CF_DEBUGS
	debugprintf("rex: failed all authorization attempts\n") ;
#endif

	goto badrexec ;

/* we got in! */
goodsupply:
goodrcmdu:
	if (mpp != NULL)
	    *mpp = NULL ;

	goto done ;

goodrexec:
	if (mpp != NULL)
	    *mpp = mp ;

done:

#if	CF_DEBUGS
	debugprintf("rex: finishing up\n") ;
#endif

	return rfd ;

/* bad returns come here */
badret:
	return srs ;

badtoomuch:
	srs = SR_2BIG ;
	goto badret ;

badhost:
	srs = SR_INVAL ;
	goto badret ;

badprog:
	srs = SR_INVAL ;
	goto badret ;

badrexec:
	srs = SR_ACCES ;
	goto badret ;

hostdown:
	srs = rs ;
	goto badret ;

badunreach:
	srs = SR_HOSTUNREACH ;
	goto badret ;

}
/* end subroutine (rex) */


/* local subroutines */


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

	Synopsis:


**************************************************************************/


/* compare host names */
static int hostequiv(h1,h2,localdomain)
char	h1[], h2[] ;
char	localdomain[] ;
{
	int	f_h1 = FALSE, f_h2 = FALSE ;
	int	len1, len2 ;

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

	    if (len1 != len2) return FALSE ;

	    cp1 += 1 ;
	    if (strcasecmp(cp1,localdomain) != 0) return FALSE ;

	    return (strncasecmp(h1,h2,len1) == 0) ;

	} /* end if */

	len1 = strlen(h1) ;

	len2 = cp2 - h2 ;
	if (len1 != len2) return FALSE ;

	cp2 += 1 ;
	if (strcasecmp(cp2,localdomain) != 0) return FALSE ;

	return (strncasecmp(h1,h2,len2) == 0) ;
}
/* end subroutine (hostequiv) */


