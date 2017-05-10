/* rcommand */

/* subroutine to return a socket to a remote command */
/* last modified %G% versiob %I% */


/* revision history:

	= 92/04/10, David A­D­ Morano

	This subroutine was first written and I probably took
	the beginnings of it from some other program that I
	previously wrote that did something similar.


*/



#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/socket.h>
#include	<sys/in.h>
#include	<signal.h>
#include	<netdb.h>
#include	<errno.h>

#include	"localmisc.h"



/* local defines */

#define	CONNTIMEOUT	120
#define	READTIMEOUT	30
#define	NETTIMELEN	4
#define	BUFLEN		100



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;


/* external variables */





int rcommand(hostname,locuser,remuser,command,fd2p)
char	hostname[] ;
char	*locuser ;
char	*remuser ;
char	*command ;
int	*fd2p ;
{
	struct sockaddr_in	server ;

	struct sockaddr		from ;

	int		i, len, rs ;
	int		s ;
	int		lulen, rulen, clen ;

	char		buf[BUFLEN] ;
	char		*cp ;


/* check arguments */

	lulen = strlen(locuser) ;

	rulen = strlen(remuser) ;

	clen = strlen(command) ;

	if ((lulen == 0) || (rulen == 0) || (clen == 0))
	    return BAD ;


/* look up some miscellaneous stuff in various databases */

	sp = getservbyname("shell", "tcp")) ;

	if (sp == ((struct servent *) 0)) {

	    sp = getservbyname("cmd", "tcp")) ;

	    if (sp == ((struct servent *) 0)) {

	        sp = getservbyname("rsh", "tcp")) ;

	        if (sp == ((struct servent *) 0))
	            return BAD ;

	    }
	}

	if ((pp = getprotobyname("tcp")) == ((struct protoent *) 0))
	    return BAD ;


/* look up the hosts given us and try to connect to one of them */

#if	ALARM
	signal(SIGALRM,timeout) ;
#endif

	for (i = 0 ; i < pan ; i += 1) {

	    if (f_debug) {

	        bprintf(efp,"%s: trying host '%s'\n",
	            progname,hostname[i]) ;

	    }

	    if ((hp = gethostbyname(hostname[i])) == ((struct hostent *) 0)) {

	        bprintf(efp,"%s: host '%s' not in hosts database\n",
	            progname,hostname[i]) ;

	        continue ;
	    }

	    if (f_debug) {

	        bprintf(efp,"%s: looked up the host\n",
	            progname) ;

	        bflush(efp) ;

	    }

/* create a socket */

	    if ((s = socket(PF_INET, SOCK_STREAM, pp->p_proto)) == 0) {

	        bprintf(efp,"%s: socket problem (%d)",
	            progname,s) ;

	        goto done ;
	    }

	    if (f_debug) {

	        bprintf(efp,"%s: created a socket \n",
	            progname) ;

	        bflush(efp) ;

	    }

	    bzero((char *) &server, sizeof(server)) ;

	    server.sin_port = sp->s_port ;
	    bcopy(hp->h_addr, (char *) &server.sin_addr, hp->h_length) ;

	    server.sin_family = hp->h_addrtype ;

/* connect to the host */

#if	ALARM
	    alarm(CONNTIMEOUT) ;
#endif

	    if ((rs = connect(s, &server, sizeof(server))) < 0) {

	        bprintf(efp,"%s: connect problem (errno %d)\n",
	            progname,errno) ;

	        close(s) ;

	        continue ;
	    }

	    if (f_debug) {

	        bprintf(efp,"%s: connected to the time server host\n",
	            progname) ;

	        bflush(efp) ;

	    }

#if	ALARM
	    alarm(READTIMEOUT) ;
#endif

	    if ((rs = read(s,buf,NETTIMELEN)) != NETTIMELEN) {

	        if (rs == EINTR) bprintf(efp,"%s: read interrupt\n",
	            progname) ;

	        else if (rs < 0) bprintf(efp,"%s: read error (%d)\n",
	            progname,rs) ;

	        else bprintf(efp,"%s: didn't read enough from socket\n",
	            progname) ;

	        close(s) ;

	        continue ;
	    }

	    close(s) ;

	    break ;

	} /* end for */

	if (i >= pan) {

	    bprintf(efp,"%s: no more hosts to try - exting\n",
	        progname) ;

	    goto done ;
	}

	alarm(0) ;

	memcpy(&lw,buf,4) ;

	newclock = ntohl(lw) ;





}
/* end subroutine (rcommand) */



