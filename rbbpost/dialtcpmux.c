/* dialtcpmux */

/* dial out to a machine server listening on TCPMUX */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_CR		1		/* stupid CR character */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will dial out to an INET host using the TCPMUX protocol
	and the optional TCP port and the TCPMUX service that is specified.

	Synopsis:

	int dialtcpmux(hostname,portspec,af,svcbuf,sargs,to,opts)
	const char	hostname[] ;
	const char	portspec[] ;
	int		af ;
	const char	svcbuf[] ;
	const char	*sargs[] ;
	int		to, opts ;

	Arguments:

	hostname	host to dial to
	portspec	port specification to use
	af		address family
	svcbuf		service specification
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
	character in addition to a normal NL character at the end of the
	service string in the dial sequence.  Many stupid TCPMUX servers out
	there will not work otherwise.


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


/* local defines */

#define	VERSION		"0"
#define	NTRIES		2
#define	BUFLEN		(8 * 1024)

#ifndef	LOGIDLEN
#define	LOGIDLEN	14
#endif

#define	LOGTIMEOUT	4		/* seconds */

#ifndef	PORTSPEC_TCPMUX
#define	PORTSPEC_TCPMUX		"tcpmux"
#endif
#define	PORTSPEC_TCPMUXALT	"5108"

#define	QBUFLEN		(MAXNAMELEN +20)
#define	RBUFLEN		LINEBUFLEN


/* external subroutines */

extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	mkquoted(char *,int,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int	getsvclen(cchar *) ;
static int	getmuxlen(int,cchar **) ;
static int	mkmuxreq(char *,int,cchar *,int,cchar **) ;


/* local variables */


/* exported subroutines */


int dialtcpmux(hostname,portspec,af,svcbuf,sargs,to,opts)
const char	hostname[] ;
const char	portspec[] ;
int		af ;
const char	svcbuf[] ;
const char	*sargs[] ;
int		to ;
int		opts ;
{
	int		rs ;
	int		svclen ;
	int		muxlen ;
	int		fd = -1 ;
	char		*muxbuf ;

	if (hostname == NULL) return SR_FAULT ;
	if (svcbuf == NULL) return SR_FAULT ;

	if (hostname[0] == '\0') return SR_INVALID ;
	if (svcbuf[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("dialtcpmux: hostname=%s portname=%s svcbuf=%s\n",
	    hostname,portspec,svcbuf) ;
	debugprintf("dialtcpmux: ent to=%d\n",to) ;
#endif

	while (CHAR_ISWHITE(*svcbuf)) {
	    svcbuf += 1 ;
	}

	svclen = getsvclen(svcbuf) ;

	muxlen = getmuxlen(svclen,sargs) ;

#if	CF_DEBUGS
	debugprintf("dialtcpmux: final svcbuf=%t\n",svcbuf,svclen) ;
#endif

	if ((rs = uc_malloc((muxlen+1),&muxbuf)) >= 0) {
	    if ((rs = mkmuxreq(muxbuf,muxlen,svcbuf,svclen,sargs)) >= 0) {
		struct sigaction	sighand, osighand ;
		sigset_t		signalmask ;
	        const int		mlen = rs ;

/* continue */

	        uc_sigsetempty(&signalmask) ;

	        memset(&sighand,0,sizeof(struct sigaction)) ;
	        sighand.sa_handler = SIG_IGN ;
	        sighand.sa_mask = signalmask ;
	        sighand.sa_flags = 0 ;

	        if ((rs = u_sigaction(SIGPIPE,&sighand,&osighand)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("dialtcpmux: service buffer, len=%d\n",mlen) ;
	            debugprintf("dialtcpmux: muxbuf=>%t<\n",
	                muxbuf,strlinelen(muxbuf,mlen,40)) ;
#endif

#if	CF_DEBUGS
	            debugprintf("dialtcpmux: about to portspec=%s\n",
	                portspec) ;
#endif

	            if ((portspec == NULL) || (portspec[0] == '\0')) {

#if	CF_DEBUGS
	                debugprintf("dialtcpmux: NULL portspec\n") ;
#endif

	                portspec = PORTSPEC_TCPMUX ;
	                rs = dialtcp(hostname,portspec,af,to,opts) ;
	                fd = rs ;

	                if ((rs < 0) && (rs != SR_NOMEM)) {

#if	CF_DEBUGS
	                    debugprintf("dialtcpmux: second chance\n") ;
#endif

	                    portspec = PORTSPEC_TCPMUXALT ;
	                    rs = dialtcp(hostname,portspec,af,to,opts) ;
	                    fd = rs ;

	                } /* end if */

#if	CF_DEBUGS
	                debugprintf("dialtcpmux: NULL-portspec rs=%d\n",rs) ;
#endif

	            } else {
	                rs = dialtcp(hostname,portspec,af,to,0) ;
	                fd = rs ;
	            }

#if	CF_DEBUGS
	            debugprintf("dialtcpmux: dialtcp() rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {

/* transmit the formatted service code and arguments */

#if	CF_DEBUGS
	                debugprintf("dialtcpmux: writing service format\n") ;
#endif

	                if ((rs = uc_writen(fd,muxbuf,mlen)) >= 0) {
	                    const int	rlen = RBUFLEN ;
	                    char	rbuf[RBUFLEN+1] = { 0 } ;

	                    if ((rs = uc_readlinetimed(fd,rbuf,rlen,to)) >= 0) {
	                        if ((rs == 0) || (rbuf[0] != '+'))
	                            rs = SR_BADREQUEST ;
	                    }

	                } /* end if (read response) */

#if	CF_DEBUGS
	                debugprintf("dialtcpmux: response rs=%d\n",rs) ;
#endif

	                if (rs < 0) u_close(fd) ;
	            } /* end if (opened) */

	            u_sigaction(SIGPIPE,&osighand,NULL) ;
	        } /* end if (signal) */

	    } else {
	        rs = SR_TOOBIG ;
	    }
	    uc_free(muxbuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("dialtcpmux: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialtcpmux) */


/* local subroutines */


static int getsvclen(cchar *svcbuf)
{
	int		svclen = strlen(svcbuf) ;
	while (svclen && CHAR_ISWHITE(svcbuf[svclen - 1])) {
	    svclen -= 1 ;
	}
	return svclen ;
}
/* end subroutine (getsvclen) */


static int getmuxlen(int svclen,cchar **sargs)
{
	int		ml = (svclen+4) ;
	if (sargs != NULL) {
	    int	i ;
	    for (i = 0 ; sargs[i] != NULL ; i += 1) {
		ml += (strlen(sargs[i])+3) ;
	    } /* end for */
	} /* end if */
	return ml ;
}
/* end subroutine (getmuxlen) */


/* format the service string to be transmitted */
static int mkmuxreq(muxbuf,muxlen,svcbuf,svclen,sargs)
char		muxbuf[] ;
int		muxlen ;
const char	svcbuf[] ;
int		svclen ;
const char	*sargs[] ;
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;

	if ((rs = sbuf_start(&b,muxbuf,muxlen)) >= 0) {
	    if ((rs = sbuf_strw(&b,svcbuf,svclen)) >= 0) {

	        if (sargs != NULL) {
	            int		i ;
	            const int	qlen = QBUFLEN ;
	            char	qbuf[QBUFLEN+1] ;
	            for (i = 0 ; (rs >= 0) && (sargs[i] != NULL) ; i += 1) {
	                sbuf_char(&b,' ') ;
	                if ((rs = mkquoted(qbuf,qlen,sargs[i],-1)) >= 0) {
	                    len = rs ;
	                    sbuf_buf(&b,qbuf,len) ;
	                }
	            } /* end for */
	        } /* end if (svc-args) */

	        if (rs >= 0) {
#if	CF_CR
	            sbuf_char(&b,'\r') ;
#endif
	            sbuf_char(&b,'\n') ;
	        } /* end if */

	    } /* end if */
	    len = sbuf_getlen(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkmuxreq) */


