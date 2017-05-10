/* renv */

/* subroutines to support sending environment over to a remote host */


#define	CF_DEBUG	1


/* revision history:

	- David A.D. Morano, 96/11/21
	These subroutines were copyied from the RSLOW program.
	(Is it the source of everything ??)

	- David A.D. Morano, 97/04/16
	These subroutines were copied from the 'RSHE' program
	for use by the 'REX' program.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Call as:


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stropts.h>
#include	<poll.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"netfile.h"


/* local defines */

#define	DISBUFLEN		(MAXHOSTNAMELEN + 50)


/* external subroutines */

extern int	getnodedomain() ;
extern int	authfile() ;
extern int	cfdec() ;

extern int	hostequiv() ;

extern char	*putheap() ;
extern char	*strshrink() ;
extern char	*strbasename() ;
extern char	*timestr_log() ;


/* forward subroutines */

static int	newbuf() ;
static int	quotevalue() ;


/* external variables */

extern struct global	g ;

extern char	**environ ;


/* local variables */





/* send over the variables in RXPORT */
int send_rxenv(jip,rxport)
struct jobinfo	*jip ;
char		*rxport ;
{
	int	klen ;
	int	rs ;

	char	*keyp = rxport ;
	char	*cp ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("send_rxenv: entered\n") ;
#endif

	if ((rxport == NULL) || (*rxport == '\0')) return OK ;

#if	CF_DEBUG
	if (g.debuglevel > 1) {

	debugprintf("send_rxenv: about to loop\n") ;

	debugprintf("send_rxenv: rxport=\"%s\"\n",rxport) ;

	}
#endif

	while ((cp = strchr(keyp,',')) != NULL) {

	    klen = cp - keyp ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("send_rxenv: var=\"%W\"\n",keyp,klen) ;
#endif

	    rs = proc_rxenv(jip,keyp,klen) ;

	    if (rs < 0) return rs ;

	    keyp = cp + 1 ;

	} /* end while */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("send_rxenv: var=\"%W\"\n",keyp,klen) ;
#endif

	klen = strlen(keyp) ;

	rs = proc_rxenv(jip,keyp,klen) ;

	return rs ;
}
/* end subroutine (send_rxenv) */


/* process the RXPORT type environment variables */
int proc_rxenv(jip,key,klen)
struct jobinfo	*jip ;
char		key[] ;
int		klen ;
{
	int	elen ;

	char	**ep ;
	char	*cp ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("proc_rxenv: entered var=\"%W\"\n",key,klen) ;
#endif

	if (klen <= 0) return OK ;

/* search for this variable name in the process environment list */

	for (ep = environ ; (*ep) != NULL ; ep += 1) {

	    if ((cp = strchr((*ep),'=')) == NULL) continue ;

	    elen = cp - (*ep) ;
	    if ((elen == klen) && (strnncmp(*ep,elen,key,klen) == 0))
	        return proc_env(jip,key,klen,cp + 1) ;

	} /* end for */

	return OK ;
}
/* end subroutine (proc_rxenv) */


/* send over the current process environment */
int send_procenv(jip)
struct jobinfo	*jip ;
{
	int	i, klen ;
	int	rs ;

	char	**ep ;
	char	*cp ;


/* write out our process environment to the remote command */

	ep = environ ;
	for (i = 0 ; ep[i] != NULL ; i += 1) {

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("send_procenv: top environment loop\n") ;
#endif

	    if ((ep[i][0] == '_') ||
	        (ep[i][0] < ' ')) continue ;

/* break it up into the two pieces */

	    if ((cp = strchr(ep[i],'=')) != NULL) {

	        klen = cp - ep[i] ;	/* length of variable name */

#if	CF_DEBUG
	if (g.debuglevel > 1) {

	        debugprintf("send_procenv: environment name=%W\n",ep[i],klen) ;

	        debugprintf("send_procenv: environment value=%s\n",cp + 1) ;

	}
#endif
	        rs = proc_env(jip,ep[i],klen,cp + 1) ;

	        if (rs < 0) return rs ;

	    } /* end if (we have a value to this variable) */

	} /* end for (sending over environment) */

	return OK ;
}
/* end subroutine (send_procenv) */


/* process an environment variable */

/*
	Arguments:
	- kp	pointer to the keyname string
	- klen	length of keyname string
	- vp	pointer to the value string
*/

int proc_env(jip,kp,klen,vp)
struct jobinfo	*jip ;
char	*kp, *vp ;
int	klen ;
{
	int	rs ;
	int	f_alloc ;

	char	*cp, *nvp ;
	char	displaybuf[BUFLEN + 1] ;
	char	valuebuf[MAXPATHLEN + 21] ;


	if (klen < 0) klen = strlen(kp) ;

/* perform some preprocessing on specific variables */

	if (strnncmp(kp,klen,"DISPLAY",7) == 0) {

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("proc_env: special DISPLAY value=%s\n",vp) ;
#endif

	    cp = (jip->f_remotedomain) ? jip->domainname : "" ;
	    rs = qualdisplay(vp,jip->nodename,cp,displaybuf,DISBUFLEN) ;

	    if (rs >= 0)
	        vp = displaybuf ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("proc_env: special DISPLAY after qualled value=%s\n",vp) ;
#endif

	} else if ((strnncmp(kp,klen,"HOME",-1) == 0) ||
	    (strnncmp(kp,klen,"PWD",-1) == 0) ||
	    (strnncmp(kp,klen,"LOGNAME",-1) == 0) ||
	    (strnncmp(kp,klen,"HZ",-1) == 0) ||
	    (strnncmp(kp,klen,"TERM",-1) == 0)) {

	    return OK ;

	} else if (strnncmp(kp,klen,"LOCALDOMAIN",-1) == 0) {

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("proc_env: got a LOCALDOMAIN variable \"%W\"\n",
	        kp,klen) ;
#endif

	    if (jip->f_remotedomain) return OK ;

	} /* end if (processing special environment variables) */

/* perform general character escape processing */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("proc_env: about to call quote\n") ;
#endif

	if ((rs = quotevalue(vp,valuebuf,MAXPATHLEN,&nvp)) < 0) {

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("proc_env: return bad alloc rs=%d\n",rs) ;
#endif

	    goto badalloc ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("proc_env: returned w/ rs=%d\n",rs) ;
#endif

	f_alloc = rs ;
	if (f_alloc) vp = nvp ;

	if (klen > 0) {

	    bprintf(jip->jfp,"%W=\"%s\"\n",
	        kp,klen, vp) ;

	    bprintf(jip->jfp,"export %W\n",
	        kp,klen) ;

	} /* end if (we have something to send over) */

	if (f_alloc) free(vp) ;

	return OK ;

badalloc:
	return BAD ;
}
/* end subroutine (proc_env) */


/* quote a string for SHELL escaping */
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

	char	*cp, *cp1 ;
	char	*curbuf = buf ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("quotevalue: entered value=%s\n",vs) ;
#endif

	cp = vs ;
	rlen = buflen ;
	blen = 0 ;
	while ((cp1 = strpbrk(cp,"\\\"'$")) != NULL) {

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("quotevalue: got one\n") ;
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

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("quotevalue: bottom of got one\n") ;
#endif

	} /* end while */

	if (f_got) {

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("quotevalue: after got one\n") ;
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

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("quotevalue: bottom after got one\n") ;
#endif

	} /* end if (we got something) */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("quotevalue: exiting f_got=%d\n",f_got) ;
#endif

	*nvpp = (f_got) ? curbuf : vs ;
#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("quotevalue: exiting for real, blen=%d\n",blen) ;
#endif

	return blen ;

badalloc:
	if (f_malloc) free(curbuf) ;

	return BAD ;
}
/* end subroutine (quotevalue) */


/* get a new buffer */
static int newbuf(curbuf,curbuflen,f,nbpp)
int	curbuflen, f ;
char	*curbuf ;
char	**nbpp ;
{
	int	newbuflen = 0 ;


	if (f) {

	    newbuflen = curbuflen * 2 ;
	    if ((*nbpp = realloc(curbuf,newbuflen)) == NULL)
	        return BAD ;

	} else {

	    newbuflen = curbuflen * 2 ;
	    if ((*nbpp = malloc(newbuflen)) == NULL)
	        return BAD ;

	}

	return newbuflen ;
}
/* end subroutine (newbuf) */



