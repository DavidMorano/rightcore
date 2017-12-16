/* dialfinger */

/* dial out to a machine server listening on FINGER */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_CR		1		/* stupid CR character */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will dial out to an INET host using the FINGER protocol
        and the optional TCP port and FINGER service that is specified.

	Synopsis:

	int dialfinger(hostname,portspec,af,svc,sargs,to,opts)
	const char	hostname[] ;
	const char	portspec[] ;
	int		af ;
	const char	svc[] ;
	const char	*sargs[] ;
	int		to, opts ;

	Arguments:

	hostname	host to dial to
	portspec	port specification to use
	af		address family
	svc		service specification
	argvargs	pointer to array of pointers to arguments
	to		to ('>0' means use one, '-1' means don't)
	opts		any dial options

	Returns:

	>=0		file descriptor
	<0		error in dialing


	What is up with the 'to' argument?

	>0		use the to as it is

	==0             asynchronously span the connect and do not wait
			for it

	<0              use the system default to (xx minutes --
			whatever)

	Notes:

        Generally, you MUST turn on the compile-time flag at the top of this
        file named 'CF_CR' in order to enable code that will transmit a CR
        character in addition to a normal NL character at the end of the service
        string in the dial sequence. Many stupid FINGER servers out there will
        not work otherwise.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<char.h>
#include	<localmisc.h>

#include	"dialopts.h"


/* local defines */

#define	VERSION		"0"
#define	NTRIES		2
#define	BUFLEN		(8 * 1024)
#define	SERIALFILE1	"/tmp/serial"
#define	SERIALFILE2	"/tmp/dialfinger.serial"
#define	LOGFNAME	"/tmp/dialfinger.log"

#ifndef	LOGIDLEN
#define	LOGIDLEN	14
#endif

#ifndef	PORTSPEC_FINGER
#define	PORTSPEC_FINGER		"finger"
#endif
#define	PORTSPEC_FINGERALT	"5112"

#ifndef	SVCLEN
#define	SVCLEN		MAXNAMELEN
#endif

#define	MUXBUFLEN	(SVCLEN + MAXPATHLEN)
#define	QBUFLEN		(MAXNAMELEN +20)
#define	RBUFLEN		LINEBUFLEN


/* external subroutines */

extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	mkquoted(char *,int,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int	getsvclen(cchar *) ;
static int	getmlen(int,cchar **) ;
static int	dialourtcp(cchar *,cchar *,int,int,int) ;
static int	mkmuxreq(char *,int,const char *,int,const char **,int) ;


/* local variables */


/* exported subroutines */


int dialfinger(hostname,portspec,af,svc,sargs,to,opts)
const char	hostname[] ;
const char	portspec[] ;
int		af ;
const char	svc[] ;
const char	*sargs[] ;
int		to ;
int		opts ;
{
	int		rs ;
	int		svclen ;
	int		mlen ;
	int		fd = -1 ;
	char		*mbuf ;

	if (hostname == NULL) return SR_FAULT ;
	if (svc == NULL) return SR_FAULT ;

	if ((hostname[0] == '\0') || (svc[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("dialfinger: hostname=%s portname=%s svc=%s\n",
	    hostname,portspec,svc) ;
	debugprintf("dialfinger: ent to=%d\n",to) ;
#endif

/* perform some cleanup on the service code specification */

	while (CHAR_ISWHITE(*svc)) {
	    svc += 1 ;
	}

	svclen = getsvclen(svc) ;

	mlen = getmlen(svclen,sargs) ;

/* format the service string to be transmitted */

	if ((rs = uc_malloc((mlen+1),&mbuf)) >= 0) {
	    if ((rs = mkmuxreq(mbuf,mlen,svc,svclen,sargs,opts)) >= 0) {
	        struct sigaction	sighand, osighand ;
	        sigset_t		sigmask ;
	        int			ml = rs ;

	        uc_sigsetempty(&sigmask) ;

	        memset(&sighand,0,sizeof(struct sigaction)) ;
	        sighand.sa_handler = SIG_IGN ;
	        sighand.sa_mask = sigmask ;
	        sighand.sa_flags = 0 ;

	        if ((rs = u_sigaction(SIGPIPE,&sighand,&osighand)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("dialfinger: service buffer ml=%d\n",ml) ;
	            debugprintf("dialfinger: mbuf=>%t<\n",
	                mbuf,strlinelen(mbuf,ml,40)) ;
	            debugprintf("dialfinger: about to portspec=%s\n",
	                portspec) ;
#endif

	            if ((rs = dialourtcp(hostname,portspec,af,to,opts)) >= 0) {
	                fd = rs ;
	                if ((rs = uc_writen(fd,mbuf,ml)) >= 0) {
	                    rs = u_shutdown(fd,SHUT_WR) ;
	                }
	                if (rs < 0) u_close(fd) ;
	            } /* end if (opened) */

	            u_sigaction(SIGPIPE,&osighand,NULL) ;
	        } /* end if (signal) */

	    } else {
	        rs = SR_TOOBIG ;
	    }
	    uc_free(mbuf) ;
	} /* end if (memory-allocation-free) */

#if	CF_DEBUGS
	debugprintf("dialfinger: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialfinger) */


/* local subroutines */


static int getsvclen(cchar *svc)
{
	int		sl = strlen(svc) ;
	while (sl && CHAR_ISWHITE(svc[sl- 1])) {
	    sl -= 1 ;
	}
	return sl ;
}
/* end subroutine (getsvclen) */


static int getmlen(int svclen,cchar **sargs)
{
	int		ml = (svclen+4) ;
	if (sargs != NULL) {
	    int		i ;
	    for (i = 0 ; sargs[i] != NULL ; i += 1) {
	        ml += (strlen(sargs[i])+3) ;
	    } /* end for */
	} /* end if */
	return ml ;
}
/* end subroutine (getmlen) */


static int dialourtcp(cchar *hs,cchar *ps,int af,int to,int opt)
{
	int		rs = SR_OK ;
	int		fd = -1 ;
#if	CF_DEBUGS
	debugprintf("dialourtcp: ent hs=%s\n",hs) ;
	debugprintf("dialourtcp: ps=%s\n",ps) ;
#endif
	if ((ps == NULL) || (ps[0] == '\0')) {
	    ps = PORTSPEC_FINGER ;
	    rs = dialtcp(hs,ps,af,to,opt) ;
	    fd = rs ;
#if	CF_DEBUGS
	    debugprintf("dialourtcp: 1 dialtcp() rs=%d\n",rs) ;
#endif
	    if ((rs < 0) && (rs != SR_NOMEM)) {
	        ps = PORTSPEC_FINGERALT ;
	        rs = dialtcp(hs,ps,af,to,opt) ;
	        fd = rs ;
	    } /* end if */
	} else {
	    rs = dialtcp(hs,ps,af,to,opt) ;
	    fd = rs ;
#if	CF_DEBUGS
	    debugprintf("dialourtcp: 2 dialtcp() rs=%d\n",rs) ;
#endif
	}
#if	CF_DEBUGS
	debugprintf("dialourtcp: ret rs=%d fd=%u\n",rs,fd) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialourtcp) */


/* format the service code and arguments for transmission */
static int mkmuxreq(mbuf,mlen,svc,svclen,sargs,opts)
char		mbuf[] ;
int		mlen ;
const char	svc[] ;
int		svclen ;
const char	*sargs[] ;
int		opts ;
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;

	if ((rs = sbuf_start(&b,mbuf,mlen)) >= 0) {
	    if ((rs = sbuf_strw(&b,svc,svclen)) >= 0) {

	        if (opts & DIALOPT_LONG) {
	            rs = sbuf_strw(&b," /W",3) ;
	        }

	        if ((rs >= 0) && (sargs != NULL)) {
	            const int	qlen = QBUFLEN ;
	            char	qbuf[QBUFLEN+1] ;
	            int		i ;
	            for (i = 0 ; (rs >= 0) && (sargs[i] != NULL) ; i += 1) {
	                sbuf_char(&b,' ') ;
	                if ((rs = mkquoted(qbuf,qlen,sargs[i],-1)) >= 0) {
	                    int	ql = rs ;
	                    sbuf_buf(&b,qbuf,ql) ;
	                }
	            } /* end for */
	        } /* end if */

#if	CF_DEBUGS
	        debugprintf("dialfinger: finishing service format\n") ;
#endif

	        if (rs >= 0) {
#if	CF_CR
	            sbuf_char(&b,'\r') ;
#endif
	            sbuf_char(&b,'\n') ;
	        } /* end if */

	    } /* end if */
	    len = sbuf_getlen(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkmuxreq) */


