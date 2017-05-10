/* rexecl */

/* subroutine to return a socket to a remote command */
/* last modified %G% version %I% */


#define	F_DEBUG		0
#define	F_PARTIAL	0


/*
	Dave Morano, April 1992
	This subroutine was originally written.


*/



#define	_REENTRANT	1
#define	F_ALARM		0



#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<signal.h>
#include	<netdb.h>
#include	<errno.h>
#include	<crypt.h>

#include	"misc.h"



/* defines */

#define		NTRIES		4
#define		CONNTIMEOUT	120
#define		READTIMEOUT	30
#define		NETTIMELEN	4
#define		BUFLEN		(2 * MAXPATHLEN)
#define		DEFEXECSERVICE	512



/* external subroutines */

extern int	cfdec() ;


/* external variables */

extern int	errno ;


/* forward references */

static int	makeconn() ;



int rexecl(ahost,port,username,password,command,fd2p)
char	**ahost ;
int	port ;
char	username[] ;
char	password[] ;
char	command[] ;
int	*fd2p ;
{
	struct sockaddr_in	server ;

	struct sockaddr_in	from ;

	struct servent	se, *sp ;

	struct protoent	pe, *pep ;

	struct hostent	he, *hep ;

	int		i, len, rs ;
	int		s1 ;
	int		ulen, plen ;
	int		proto ;
	int		h_errno ;
	int		f_err ;

	char		buf[BUFLEN + 1] ;


#if	F_DEBUG
	eprintf("rexecl: entered\n") ;
#endif

/* check arguments */

	if (ahost == NULL) return BAD ;

	if (*ahost == NULL) return BAD ;

	if ((*ahost)[0] == '\0') return BAD ;

	if ((username == NULL) || (username[0] == '\0')) return BAD ;

	if (password == NULL) password = "" ;

	if ((ulen = strlen(username)) > 16) return BAD ;

	if ((plen = strlen(password)) > 16) return BAD ;

	if ((command == NULL) || (command[0] == '\0')) return BAD ;


/* look up some miscellaneous stuff in various databases */

	if (port < 0) {

#ifdef	SYSV
	    sp = getservbyname_r("exec", "tcp",
	        &se,buf,BUFLEN) ;
#else
	    sp = getservbyname("exec", "tcp") ;
#endif

	    if (sp == NULL) {

#ifdef	SYSV
	        sp = getservbyname_r("rexec", "tcp",
	            &se,buf,BUFLEN) ;
#else
	        sp = getservbyname("rexec", "tcp") ;
#endif

	    }

	    if (sp != NULL)
	        port = (int) ntohs(sp->s_port) ;

	    else
	        port = DEFEXECSERVICE ;

	} /* end if (no port specified) */


/* get the protocol number */

#if	SYSV
	pep = getprotobyname_r("tcp",
	    &pe,buf,BUFLEN) ;
#else
	pep = getprotobyname("tcp") ;
#endif

	if (pep != NULL)
	    proto = pep->p_proto ;

	else
	    proto = 6 ;


/* look up the host name given us */

#if	F_ALARM
	signal(SIGALRM,timeout) ;
#endif

#if	F_DEBUG
	eprintf("rexecl: trying host '%s'\n",
	    *ahost) ;
#endif

#ifdef	SYSV
	h_errno = 0 ;
	do {

	    hep = gethostbyname_r(*ahost,
	        &he,buf,BUFLEN,&h_errno) ;

	    if ((hep == NULL) && (h_errno == TRY_AGAIN)) sleep(1) ;

	} while ((hep == NULL) && (h_errno == TRY_AGAIN)) ;
#else
	do {

	    hep = gethostbyname(*ahost) ;

	    if ((hep == NULL) && (h_errno == TRY_AGAIN)) sleep(1) ;

	} while ((hep == NULL) && (h_errno == TRY_AGAIN)) ;
#endif


	if (hep == NULL) {

#if	F_DEBUG
	    eprintf("rexecl: host '%s' not in hosts database\n",
	        *ahost) ;
#endif

	    return BAD ;
	}


/* server address */

	memset((char *) &server,0,sizeof(struct sockaddr_in)) ;

	server.sin_family = hep->h_addrtype ;
	server.sin_port = htons(port) ;

	memcpy((char *) &server.sin_addr, hep->h_addr, hep->h_length) ;

/* from address */

	memset((char *) &from,0,sizeof(struct sockaddr_in)) ;

	from.sin_family = hep->h_addrtype ;

/* other */

	f_err = FALSE ;
	if (fd2p != NULL) f_err = TRUE ;

/* try to connect to the remote machine */

	rs = 1 ;
	for (i = 0 ; (rs > 0) && (i < NTRIES) ; i += 1) {

	    rs = makeconn(&server,&from,proto, username,password,command,
	        &s1,fd2p,f_err) ;

	} /* end for */

	if (i >= NTRIES) goto noconnect ;

#if	F_ALARM
	alarm(0) ;
#endif

	return (rs == 0) ? s1 : rs ;

noconnect:
	return BAD ;
}
/* end subroutine (rexecl) */


static int makeconn(sp,fp,proto,username,password,command,s1p,s2p,f_err)
struct sockaddr_in	*sp ;
struct sockaddr_in	*fp ;
int	proto ;
char	username[], password[], command[] ;
int	*s1p, *s2p ;
int	f_err ;
{
	int	localport = 0 ;
	int	slisten ;
	int	s1 ;
	int	s2 ;
	int	i, rs, len ;
	int	f_keepalive = TRUE ;

	char	buf[BUFLEN + 1] ;


#if	F_DEBUG
	eprintf("makeconn: entered\n") ;
#endif

/* create the primary socket */

	if ((s1 = socket(AF_INET, SOCK_STREAM, proto)) < 0) {

#if	F_DEBUG
	    eprintf("makeconn: socket problem (errno %d)",
	        errno) ;
#endif

	    goto err2 ;
	}

#if	F_DEBUG
	eprintf("makeconn: created a socket \n") ;
#endif

	if ((rs = setsockopt(s1,SOL_SOCKET,SO_KEEPALIVE,
	    (const char *) &f_keepalive,sizeof(int))) < 0) goto err2 ;

#if	F_DEBUG
	eprintf("makeconn: set option on the socket \n") ;
#endif

/* attempt to connect to the host */

#if	F_ALARM
	alarm(CONNTIMEOUT) ;
#endif

	if ((rs = connect(s1,(struct sockaddr *) sp, 
	    sizeof(struct sockaddr))) < 0) {

#if	F_DEBUG
	    eprintf("makeconn: connect problem (errno %d)\n",
	        errno) ;
#endif

	    goto err2 ;
	}

#if	F_DEBUG
	eprintf("makeconn: connected initial socket\n") ;
#endif

/* bind our error channel socket */

	if (f_err) {

/* first find a free port */

#if	F_DEBUG
	    eprintf("makeconn: about to bind error socket\n") ;
#endif

	    rs = -1 ;
	    for (i = 1025 ; (rs < 0) && (i < 5000) ; i += 1) {

#if	F_DEBUG
	        eprintf("makeconn: creating an error socket, port=%d\n",i) ;
#endif

	        if ((slisten = socket(AF_INET, SOCK_STREAM, proto)) < 0)
	            goto err2 ;

#if	F_DEBUG
	        eprintf("makeconn: created second socket, port=%d\n",i) ;
#endif

	        fp->sin_port = htons(i) ;

	        if ((rs = bind(slisten, (struct sockaddr *) fp, 
	            sizeof(struct sockaddr))) >= 0) break ;

#if	F_DEBUG
	        eprintf("makeconn: bind problem port=%d (errno %d)\n",
	            i,errno) ;
#endif

	        close(slisten) ;

	    } /* end for */

	    localport = i ;

#if	F_DEBUG
	    eprintf("makeconn: done binding port=%d (rs %d errno %d)\n",
	        localport,rs,errno) ;
#endif

	    if (rs < 0) goto err3 ;


#if	F_DEBUG && 0
	    system("/home/dam/src/rexec1a/ns") ;
	    eprintf("makeconn: about to listen\n") ;
#endif

	    rs = listen(slisten,2) ;

#if	F_DEBUG
	    eprintf("makeconn: listened with (rs %d errno %d)\n",rs,errno) ;
#endif

	    if (rs < 0) goto err3 ;

#if	F_DEBUG && 0
	    system("/home/dam/src/rexec1a/ns") ;
#endif

	} /* end if (binding our error channel socket) */

/* write the port that we got above to the server */

#if	F_DEBUG
	eprintf("makeconn: writing information to server\n") ;
#endif

#if	F_PARTIAL
	len = sprintf(buf,"%d%c",
	    localport,0,username,0,password,0,command,0) ;
#else
	len = sprintf(buf,"%d%c%s%c%s%c%s%c",
	    localport,0,username,0,password,0,command,0) ;
#endif

#if	F_DEBUG
	eprintf("makeconn: sent to server >%W<\n",buf,len) ;
	for (i = 0 ; i < len ; i += 1)
	    eprintf(" %02X",buf[i]) ;
	eprintf("\n") ;
#endif

	i = 0 ;
	while ((i < len) && 
	    ((rs = write(s1,buf + i,len - i)) >= 0)) {

	    i += rs  ;

	} /* end while */

#if	F_DEBUG
	eprintf("makeconn: done writing len=%d (rs %d errno %d)\n",
	    i,rs,errno) ;
#endif

	if (rs < 0) goto err3 ;

#if	F_ALARM
	alarm(READTIMEOUT) ;
#endif

/* if we have an error channel, accept the return connection on it */

	if (f_err) {

#if	F_DEBUG
	    eprintf("makeconn: about to accept\n") ;
#endif

	    len = sizeof(struct sockaddr) ;
	    s2 = accept(slisten,(struct sockaddr *) fp,&len) ;

#if	F_DEBUG
	    eprintf("makeconn: accepted on w/ socket %d (errno %d)\n",
	        s2,errno) ;
#endif

		if (s2 < 0) goto err3 ;

	    if ((rs = setsockopt(s2,SOL_SOCKET,SO_KEEPALIVE,
	        (const char *) &f_keepalive,sizeof(int))) < 0) goto err4 ;

#if	F_DEBUG
	    eprintf("makeconn: set option on the error socket \n") ;
#endif

	} /* end if (accepting the error socket connection) */

#if	F_PARTIAL

/* send the rest to the server */

#if	F_DEBUG
	len = sprintf(buf,"%s%c%s%c%s%c",
	    username,0,password,0,command,0) ;
#endif

#if	F_DEBUG
	eprintf("makeconn: sent to server >%W<\n",buf,len) ;
	for (i = 0 ; i < len ; i += 1)
	    eprintf(" %02X",buf[i]) ;
	eprintf("\n") ;
#endif

	i = 0 ;
	while ((i < len) && 
	    ((rs = write(s1,buf + i,len - i)) >= 0)) {

	    i += rs  ;

	} /* end while */

#if	F_DEBUG
	eprintf("makeconn: done writing len=%d (rs %d errno %d)\n",
	    i,rs,errno) ;
#endif

	if (rs < 0) goto err4 ;

#endif /* PARTIAL */

/* read the result BYTE from the server on the INITIAL socket */

#if	F_DEBUG
	eprintf("makeconn: about to read socket\n") ;
#endif

	while ((rs = read(s1, buf,1)) == 0) ;

#if	F_DEBUG
	eprintf("makeconn: read socket w/ rs %d errno %d\n",
	    rs,errno) ;
#endif

	if (rs <= 0) goto err4 ;

/* read back the error if there is one */

	if (buf[0] != 0) {

#if	F_DEBUG
	    eprintf("makeconn: we got a REXEC error from server\n") ;
#endif

	    i = 0 ;
	    while ((rs = read(s1, buf + i,BUFLEN - 1)) > 0) i += rs ;

	    buf[i] = '\0' ;

#if	F_DEBUG
	    eprintf("makeconn: error \"%W\"\n",buf,i) ;
#endif


	    if (strncmp(buf,"Try again",9) == 0)
	        goto err5 ;

	    else
	        goto err4 ;

	} /* end if (error from remote server) */

	*s1p = s1 ;
	if (f_err) {

	    *s2p = s2 ;
	    close(slisten) ;

	}

	return OK ;

/* the only recoverable error */
err5:
	if (f_err) close(s2) ;

	if (f_err) close(slisten) ;

	close(s1) ;

	return 1 ;

/* handle the errors */
err4:
	if (f_err) close(s2) ;

#if	F_DEBUG
	eprintf("makeconn: err4\n") ;
#endif

err3:

#if	F_DEBUG
	eprintf("makeconn: err3\n") ;
#endif

	if (f_err) close(slisten) ;

err2:

#if	F_DEBUG
	eprintf("makeconn: err2\n") ;
#endif

	close(s1) ;

err1:
	return BAD ;
}
/* end subroutine (makeconn) */


