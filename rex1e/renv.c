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
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>
#include	<stropts.h>
#include	<poll.h>
#include	<errno.h>

#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"netfile.h"
#include	"bufstr.h"
#include	"config.h"
#include	"defs.h"


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
	        (((unsigned int) ep[i][0]) < ' ')) continue ;

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
	struct bufstr	value ;

	int	rs, l ;

	char	*cp, *nvp ;
	char	displaybuf[BUFLEN + 1] ;


#if	CF_DEBUG
	if (g.debuglevel > 2)
	debugprintf("proc_env: entered, value=%s\n",vp) ;
#endif

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

	rs = OK ;
	if (klen > 0) {

/* perform general character escape processing */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("proc_env: about to call quote\n") ;
#endif

	    bufstr_start(&value) ;

	    if ((rs = quotevalue(vp,&value)) >= 0) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("proc_env: returned w/ rs=%d\n",rs) ;
#endif

	        l = bufstr_get(&value,&nvp) ;

#if	CF_DEBUG
	if (g.debuglevel > 2)
	debugprintf("proc_env: sending value=%s\n",nvp) ;
#endif

	        bprintf(jip->jfp,"%W=\"%W\"\n",
	            kp,klen, nvp,l) ;

	        bprintf(jip->jfp,"export %W\n",
	            kp,klen) ;

	    }

	    bufstr_finish(&value) ;

	} /* end if (we have something to send over) */

	return rs ;
}
/* end subroutine (proc_env) */


/* quote a string for SHELL escaping */
static int quotevalue(vs,bsp)
char		vs[] ;
struct bufstr	*bsp ;
{
	int	rs, f_got = FALSE ;

	char	*cp, *cp1 ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("quotevalue: entered value=%s\n",vs) ;
#endif

	rs = 0 ;
	cp = vs ;
	while ((cp1 = strpbrk(cp,"\\\"'$")) != NULL) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("quotevalue: got one\n") ;
#endif

	    f_got = TRUE ;
	    bufstr_buf(bsp,cp,cp1 - cp) ;

	    bufstr_char(bsp,'\\') ;

	    rs = bufstr_char(bsp,*cp1++) ;

	    if (rs < 0) return BAD ;

	    cp = cp1 ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("quotevalue: bottom of got one\n") ;
#endif

	} /* end while */

	if (f_got)
	    rs = bufstr_buf(bsp,cp,-1) ;

	else
	    rs = bufstr_buf(bsp,vs,-1) ;

	return rs ;
}
/* end subroutine (quotevalue) */



