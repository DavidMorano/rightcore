/* rexecl */

/* subroutine to return a socket to a remote command */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_LOG		1
#define	CF_LOG2		1
#define CF_LINGER	1


/* revision history:

	= 1999-02-04, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a look-alike knock-off for the old Berkely 'rexec(3)'
	subroutine.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<netdb.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<bfile.h>
#include	<localmisc.h>


/* local defines */

#define	VERSION		"0"
#define	TIMEOUT		10
#define	NTRIES		2
#define	CONNTIMEOUT	120
#define	READTIMEOUT	30
#define	CMDLEN		(64 * 1024)
#define	BUFLEN		(8 * 1024)
#define	DEFEXECSERVICE	512
#define	LINGERTIME	(3 * 6)
#define	SERIALFILE1	"/tmp/serial"
#define	SERIALFILE2	"/tmp/rexecl.serial"
#define	LOGFNAME	"/tmp/rexecl.log"

#ifndef	LOGIDLEN
#define	LOGIDLEN	80
#endif

#ifndef	INETADDRBAD
#define	INETADDRBAD	((unsigned int) (~ 0))
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snddd(char *,int,uint,uint) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getserial(const char *) ;
extern int	getourhe(const char *,char *,struct hostent *,char *,int) ;


/* external variables */


/* local structures */

struct global_flags {
	uint		log:1 ;
} ;

struct global {
	logfile		lh ;
	struct global_flags	f ;
} ;


/* forward references */

static int	makeconn() ;


/* exported subroutines */


int rexecl(ahost,port,username,password,command,fd2p)
char		**ahost ;
int		port ;
const char	username[] ;
const char	password[] ;
const char	command[] ;
int		*fd2p ;
{
	struct protoent	pe, *pep ;

	struct hostent	he, *hep ;

	struct global	g ;

	struct sockaddr_in	server ;

	struct sockaddr_in	from ;

	struct servent	se, *sp ;

#if	CF_LOG
	bfile	sf, *sfp = &sf ;
#endif

	unsigned long	addr ;

#if	CF_LOG
	pid_t	pid = getpid() ;
	uid_t	uid ;
#endif

	int	rs = SR_OK ;
	int	rs1 ;
	int	i, srs ;
	int	s1 ;
	int	ulen, plen ;
	int	proto ;
	int	cmdlen ;
	int	f_err ;

#if	CF_LOG
	int	serial ;
#endif

	char	buf[BUFLEN + 1] ;

#if	CF_LOG
	char	logid[LOGIDLEN + 1] ;
#endif


#if	CF_DEBUGS
	debugprintf("rexecl: ent\n") ;
#endif

/* handle logging if it is enable */

#if	CF_LOG
	memset(&g,0,sizeof(struct global)) ;

	serial = getserial(SERIALFILE1) ;

#if	CF_DEBUGS
	debugprintf("getchostname: using serial=%d\n",serial) ;
#endif

	{
	    int	lid = (int) pid ;
	    snddd(logid,LOGIDLEN,lid,serial) ;
	}

	if (u_access(LOGFNAME,W_OK) >= 0) {

	    rs1 = logfile_open(&g.lh,LOGFNAME,0,0666,logid) ;

	    if (rs1 >= 0) {

	        g.f.log = TRUE ;
	        uid = getuid() ;

	        logfile_printf(&g.lh,"v=\"%s\" uid=%d\n",VERSION,uid) ;

	        logfile_printf(&g.lh,
			"ahost=%s port=%d u=%s\n",
			*ahost,port,username) ;

	    } /* end if */

	}
#endif /* CF_LOG */

/* check arguments */

	if (ahost == NULL) {

	    srs = SR_INVAL ;
	    goto badret ;
	}

	if (*ahost == NULL) {

	    srs = SR_INVAL ;
	    goto badret ;
	}

	if ((*ahost)[0] == '\0') {

	    srs = SR_INVAL ;
	    goto badret ;
	}

	if ((username == NULL) || (username[0] == '\0')) {

	    srs = SR_INVAL ;
	    goto badret ;
	}

	if (password == NULL)
	    password = "" ;

	if ((ulen = strlen(username)) > 16) {

	    srs = SR_PROTO ;
	    goto badret ;
	}

	if ((plen = strlen(password)) > 16) {

	    srs = SR_PROTO ;
	    goto badret ;
	}

	if ((command == NULL) || (command[0] == '\0')) {

	    srs = SR_INVAL ;
	    goto badret ;
	}

	if ((cmdlen = strlen(command)) > CMDLEN) {

	    srs = SR_2BIG ;
	    goto badret ;
	}

/* look up some miscellaneous stuff in various databases */

	if (port < 0) {

#if	defined(OSNAME_SunOS)
	    sp = getservbyname_r("exec", "tcp", &se,buf,BUFLEN) ;
#else
	    sp = getservbyname("exec", "tcp") ;
#endif

	    if (sp == NULL) {

#if	defined(OSNAME_SunOS)
	        sp = getservbyname_r("rexec", "tcp", &se,buf,BUFLEN) ;
#else
	        sp = getservbyname("rexec", "tcp") ;
#endif

	    }

	    if (sp != NULL) {
	        port = (int) ntohs(sp->s_port) ;
	    } else
	        port = DEFEXECSERVICE ;

	} /* end if (no port specified) */

/* get the protocol number */

#if	defined(OSNAME_SunOS)
	pep = getprotobyname_r("tcp", &pe,buf,BUFLEN) ;
#else
	pep = getprotobyname("tcp") ;
#endif

	if (pep != NULL) {
	    proto = pep->p_proto ;
	} else
	    proto = IPPROTO_TCP ;

/* look up the host name given us */

#if	CF_DEBUGS
	debugprintf("rexecl: trying host '%s'\n",
	    *ahost) ;
#endif

/* server address */

	memset(&server,0,sizeof(struct sockaddr_in)) ;

	addr = inet_addr(*ahost) ;
	if (addr == INETADDRBAD) {

	    hep = &he ;
	    if (getourhe(*ahost,NULL,hep,buf,BUFLEN) < 0) {
	        srs = SR_HOSTUNREACH ;
	        goto badret ;
	    }

	    server.sin_family = hep->h_addrtype ;
	    memcpy((char *) &server.sin_addr, hep->h_addr, hep->h_length) ;

	} else {

	    server.sin_family = htons(AF_INET) ;
	    memcpy((char *) &server.sin_addr, &addr, sizeof(long)) ;

	} /* end if */

	server.sin_port = htons(port) ;

/* from address */

	memset(&from,0,sizeof(struct sockaddr_in)) ;

	from.sin_family = htons(AF_INET) ;

/* other */

	f_err = FALSE ;
	if (fd2p != NULL) f_err = TRUE ;

/* try to connect to the remote machine */

	srs = SR_AGAIN ;
	for (i = 0 ; i < (NTRIES - 1) ; i += 1) {

	    srs = makeconn(&g,&server,&from,proto, 
	        username,password,command, &s1,fd2p,f_err) ;

	    if (srs != SR_AGAIN)
	        break ;

#if	CF_LOG2
	    if (g.f.log)
	        logfile_printf(&g.lh,"trying again %d\n",i) ;
#endif

	    sleep(1) ;

	} /* end for */

	if (srs == SR_AGAIN) {

#if	CF_LOG2
	    if (g.f.log)
	        logfile_printf(&g.lh,"trying final time %d\n",i) ;
#endif

	    srs = makeconn(&g,&server,&from,proto, 
	        username,password,command,
	        &s1,fd2p,f_err) ;

	}

#if	CF_DEBUGS
	debugprintf("rexecl: rs=%d FD=%d\n",srs,s1) ;
#endif

#if	CF_LOG
	if (g.f.log) {
	    logfile_printf(&g.lh,"rs=%d FD=%d\n",srs,s1) ;
	    logfile_close(&g.lh) ;
	}
#endif /* CF_LOG */

	return (srs >= 0) ? s1 : srs ;

badret:

#if	CF_DEBUGS
	debugprintf("rexecl: rs=%d\n",srs) ;
#endif

#if	CF_LOG
	if (g.f.log) {
	    logfile_printf(&g.lh,"rs=%d\n",srs) ;
	    logfile_close(&g.lh) ;
	}
#endif /* CF_LOG */

	return srs ;
}
/* end subroutine (rexecl) */


/* local subroutines */


static int makeconn(gp,sp,fp,proto,username,password,command,s1p,s2p,f_err)
struct global		*gp ;
struct sockaddr_in	*sp ;
struct sockaddr_in	*fp ;
int	proto ;
char	username[], password[], command[] ;
int	*s1p, *s2p ;
int	f_err ;
{
	struct linger	ls ;

	int	localport = 0 ;
	int	slisten ;
	int	s1, s2 ;
	int	rs, i, srs, len ;
	int	f_keepalive = TRUE ;
	int	timeout = 0 ;

	char	buf[BUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("makeconn: ent\n") ;
#endif

/* create the primary socket */

	if ((s1 = u_socket(PF_INET, SOCK_STREAM, proto)) < 0) {

	    srs = s1 ;

#if	CF_DEBUGS
	    debugprintf("makeconn: socket problem (rs %d)",
	        srs) ;
#endif

	    goto ret0 ;
	}

#if	CF_DEBUGS
	debugprintf("makeconn: created a socket \n") ;
#endif

	if ((srs = setsockopt(s1,SOL_SOCKET,SO_KEEPALIVE,
	    (CONST char *) &f_keepalive,sizeof(int))) < 0)
	    goto ret1 ;

#if	CF_DEBUGS
	debugprintf("makeconn: about to set LINGER\n") ;
#endif

#if	CF_LINGER
	ls.l_onoff = TRUE ;
	ls.l_linger = LINGERTIME ;
	if ((srs = setsockopt(s1,SOL_SOCKET,SO_LINGER,
	    (CONST char *) &ls,sizeof(struct linger))) < 0)
	    goto ret1 ;
#endif

#if	CF_DEBUGS
	debugprintf("makeconn: set options on the socket \n") ;
#endif

/* attempt to connect to the host */

#if	CF_LOG2
	if (gp->f.log)
	    logfile_printf(&gp->lh,"about to attempt a connect\n") ;
#endif

	if ((srs = u_connect(s1,(struct sockaddr *) sp, 
	    sizeof(struct sockaddr))) < 0) {

#if	CF_DEBUGS
	    debugprintf("makeconn: connect problem (rs %d)\n",
	        srs) ;
#endif

#if	CF_LOG
	    if (gp->f.log)
	        logfile_printf(&gp->lh,"failed connection rs=%d\n",srs) ;
#endif

	    goto ret1 ;
	}

#if	CF_DEBUGS
	debugprintf("makeconn: connected initial socket\n") ;
#endif

#if	CF_LOG2
	if (gp->f.log)
	    logfile_printf(&gp->lh,"connected initial socket\n") ;
#endif

/* bind our error channel socket */

	if (f_err) {

/* first find a free port */

#if	CF_DEBUGS
	    debugprintf("makeconn: about to bind error socket\n") ;
#endif

#if	CF_LOG2
	    if (gp->f.log)
	        logfile_printf(&gp->lh,"about to bind an error socket\n") ;
#endif

	    rs = -1 ;
	    for (i = 1024 ; (rs < 0) && (i < 5000) ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("makeconn: creating an error socket, port=%d\n",i) ;
#endif

	        rs = u_socket(PF_INET, SOCK_STREAM, proto) ;
		slisten = rs ;
		if (rs < 0) {

	            srs = slisten ;
	            goto ret1 ;
	        }

#if	CF_DEBUGS
	        debugprintf("makeconn: created second socket, port=%d\n",i) ;
#endif

	        fp->sin_port = htons(i) ;

	        rs = u_bind(slisten, (struct sockaddr *) fp, 
	            sizeof(struct sockaddr)) ;

		if (rs >= 0)
			break ;

	        srs = rs ;

#if	CF_DEBUGS
	        debugprintf("makeconn: bind problem port=%d (rs %d)\n",
	            i,rs) ;
#endif

	        u_close(slisten) ;

	    } /* end for (looping through possible ports) */

	    localport = i ;

#if	CF_DEBUGS
	    debugprintf("makeconn: done binding port=%d (rs %d)\n",
	        localport,rs) ;
#endif

#if	CF_LOG
	    if (gp->f.log)
	        logfile_printf(&gp->lh,"we bound to port=%d\n",localport) ;
#endif

	    if (rs < 0)
	        goto ret2 ;


#if	CF_DEBUGS && 0
	    system("/home/dam/src/rexec1a/ns") ;
	    debugprintf("makeconn: about to listen\n") ;
#endif

	    srs = u_listen(slisten,2) ;
	    if (srs < 0)
	        goto ret2 ;

#if	CF_DEBUGS
	    debugprintf("makeconn: listened with (rs %d)\n",rs) ;
#endif

#if	CF_DEBUGS && 0
	    system("/home/dam/src/rexec1a/ns") ;
#endif

	} /* end if (binding our error channel socket) */

/* write the port that we got above to the server */

#if	CF_DEBUGS
	debugprintf("makeconn: writing information to server\n") ;
#endif

	len = snprintf(buf,BUFLEN,"%d%c%s%c%s%c",
	    localport,0,username,0,password,0) ;

#if	CF_DEBUGS
	debugprintf("makeconn: sent to server >%W<\n",buf,len) ;
	for (i = 0 ; i < len ; i += 1)
	    debugprintf(" %02X",buf[i]) ;
	debugprintf("\n") ;
#endif

	i = 0 ;
	while ((i < len) && 
	    ((rs = u_write(s1,buf + i,len - i)) >= 0)) {

	    i += rs  ;

	} /* end while */

	if (rs < 0) {

	    srs = rs ;
	    goto ret2 ;
	}

#if	CF_DEBUGS
	debugprintf("makeconn: done writing len=%d (rs %d)\n",
	    i,rs) ;
#endif

	len = strlen(command) + 1 ;

	i = 0 ;
	while ((i < len) && 
	    ((rs = u_write(s1,command + i,len - i)) >= 0)) {

	    i += rs  ;

	} /* end while */

	if (rs < 0) {

	    srs = rs ;
	    goto ret2 ;
	}

/* if we have an error channel, accept the return connection on it */

	if (f_err) {

#if	CF_DEBUGS
	    debugprintf("makeconn: about to accept\n") ;
#endif

	    len = sizeof(struct sockaddr) ;
	    s2 = u_accept(slisten,(struct sockaddr *) fp,&len) ;

	    if (s2 < 0) {

	        srs = s2 ;
	        goto ret2 ;
	    }

#if	CF_DEBUGS
	    debugprintf("makeconn: accepted on w/ socket %d\n",
	        s2) ;
#endif

	    srs = setsockopt(s2,SOL_SOCKET,SO_KEEPALIVE,
	        (CONST char *) &f_keepalive,sizeof(int)) ;

	    if (srs < 0)
	        goto ret3 ;

#if	CF_DEBUGS
	    debugprintf("makeconn: about to set LINGER\n") ;
#endif

#if	CF_LINGER
	    ls.l_onoff = TRUE ;
	    ls.l_linger = LINGERTIME ;
	    if ((srs = setsockopt(s1,SOL_SOCKET,SO_LINGER,
	        (CONST char *) &ls,sizeof(struct linger))) < 0)
	        goto ret3 ;
#endif

#if	CF_DEBUGS
	    debugprintf("makeconn: set options on the error socket \n") ;
#endif

	} /* end if (accepting the error socket connection) */


/* read the result BYTE from the server on the INITIAL socket */

#if	CF_DEBUGS
	debugprintf("makeconn: about to read socket\n") ;
#endif

	timeout = 0 ;
	while ((rs = u_read(s1, buf,1)) == 0) {

	    timeout += 1 ;
	    if (timeout > TIMEOUT) {

	        srs = SR_TIMEDOUT ;
	        goto ret3 ;
	    }

	    sleep(1) ;

#if	CF_DEBUGS
	    debugprintf("makeconn: read zero bytes from socket\n") ;
#endif

	} /* end while (reading from the far end of socket) */

	srs = rs ;

#if	CF_DEBUGS
	debugprintf("makeconn: read socket w/ rs %dn",
	    rs) ;
#endif

	if (rs <= 0)
	    goto ret3 ;

/* read back the error if there is one */

	if (buf[0] != 0) {

#if	CF_DEBUGS
	    debugprintf("makeconn: we got a REXEC error from server\n") ;
#endif

	    i = 0 ;
	    while ((i < BUFLEN) &&
	        (rs = u_read(s1, buf + i,BUFLEN - i)) > 0)
	        i += rs ;

	    buf[i] = '\0' ;

#if	CF_DEBUGS
	    debugprintf("makeconn: error \"%W\"\n",buf,i) ;
#endif

	    srs = SR_PROTO ;
	    if (strncmp(buf,"Try again",9) == 0) {
	        srs = SR_AGAIN ;

	    } else if (strncmp(buf,"Login",5) == 0) {
	        srs = SR_ACCES ;

	    } else if (strncmp(buf,"Passw",5) == 0) {
	        srs = SR_ACCES ;

	    } else if (strncmp(buf,"No re",5) == 0)
	        srs = SR_NOTDIR ;

	    goto ret3 ;

	} /* end if (error from remote server) */

	*s1p = s1 ;
	if (f_err) {

	    *s2p = s2 ;
	    u_close(slisten) ;

	}

	return SR_OK ;

/* handle the errors */
ret3:
	if (f_err)
	    u_close(s2) ;

#if	CF_DEBUGS
	debugprintf("makeconn: ret3\n") ;
#endif

ret2:

#if	CF_DEBUGS
	debugprintf("makeconn: ret2\n") ;
#endif

	if (f_err)
	    u_close(slisten) ;

ret1:

#if	CF_DEBUGS
	debugprintf("makeconn: ret1\n") ;
#endif

	u_close(s1) ;

ret0:
	return srs ;
}
/* end subroutine (makeconn) */


