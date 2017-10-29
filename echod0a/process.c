/* process */

/* do the "echo" part of this thing ! */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif



/* external subroutines */

extern int	listenudp(int,const char *,const char *,int) ;


/* local structures */

struct sighand {
	struct sigaction	sigint ;
	struct sigaction	sigpipe ;
} ;


/* forward references */

static void	sighand_int(int) ;


/* local variables */

static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGTERM,
	SIGQUIT,
	SIGCHLD,
	0
} ;






int process(pip,hostname,portspec)
struct proginfo	*pip ;
const char	hostname[], portspec[] ;
{
	struct sockaddr from ;

	struct fd_set	fds ;

	struct sighand	nsh, osh ;

	sigset_t	oldsigmask, newsigmask ;

	const int	af = AF_INET ;

	int	rs = SR_OK ;
	int	i, len ;
	int	fromlen ;
	int	fd_stdin, fd_stdout ;
	int	ifd, ofd ;
	int	f_stream = pip->f.stream ;
	int	f_daemon = pip->f.daemon ;

	char	netbuf[BUFLEN + 1] ;


	if_int = FALSE ;

/* block some signals and catch the others */

	uc_sigsetempty(&newsigmask) ;

	for (i = 0 ; sigblocks[i] != 0 ; i += 1)
	    uc_sigsetadd(&newsigmask,sigblocks[i]) ;

	u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask) ;

/* catch or ignore these signals */

	memset(&nsh,0,sizeof(struct sighand)) ;

	uc_sigsetempty(&newsigmask) ;

	nsh.sigpipe.sa_handler = SIG_IGN ;
	nsh.sigpipe.sa_mask = newsigmask ;
	nsh.sigpipe.sa_flags = 0 ;
	u_sigaction(SIGPIPE,&nsh.sigpipe,&osh.sigpipe) ;

	nsh.sigint.sa_handler = sighand_int ;
	nsh.sigint.sa_mask = newsigmask ;
	nsh.sigint.sa_flags = 0 ;
	u_sigaction(SIGINT,&nsh.sigint,&osh.sigint) ;


	fd_stdin = FD_STDIN ;
	fd_stdout = FD_STDOUT ;

	if (f_stream) {

	    while ((! if_int) && 
	        ((rs = u_read(fd_stdin,netbuf,BUFLEN)) > 0)) {

	        len = rs ;
	        rs = u_write(fd_stdout,netbuf,len) ;

	        if (rs < 0)
	            break ;

	    } /* end while */

	} else {

	    ifd = fd_stdin ;
	    ofd = fd_stdout ;
	    if (f_daemon) {

	        rs = listenudp(af,hostname,portspec,0) ;

	        if (rs < 0)
	            goto badlisten ;

#if	CF_DEBUGS
	        fprintf(stderr,"main: listenudp() rs=%d\n",rs) ;
	        fflush(stderr) ;
#endif

	        ifd = rs ;
	        ofd = rs ;

	    } /* end if (daemon mode) */

	    while ((rs >= 0) && f_daemon && (! if_int)) {

	        fromlen = sizeof(struct sockaddr) ;
	        rs = u_recvfrom(ifd, netbuf, BUFLEN, 0, &from, &fromlen) ;

		len = rs ;
		if (len == 0)
			break ;

#if	CF_DEBUGS
	        fprintf(stderr,"main: u_recvfrom() rs=%d\n",len) ;
	        netbuf[len] = '\0' ;
	        fprintf(stderr,"main: buf=>%s<\n",netbuf) ;
	        fflush(stderr) ;
#endif

	        if ((rs >= 0) && (len >= 0)) {

	            rs = u_sendto(ofd, netbuf,
	                len, 0, &from, fromlen) ;

#if	CF_DEBUGS
	            fprintf(stderr,"main: u_sendto() rs=%d\n",rs) ;
	            fflush(stderr) ;
#endif

	        } /* end if (non-zero length) */

	    } /* end if (daemon mode) */

	} /* end if */

	if (if_int)
		rs = SR_INTR ;

ret2:
	close(fd_stdin) ;

ret1:
	close(fd_stdout) ;

#if	CF_DEBUGS
	fclose(stderr) ;
#endif

/* restore and get out */
ret0:
badlisten:
	u_sigaction(SIGINT,&osh.sigint,NULL) ;

	u_sigaction(SIGPIPE,&osh.sigpipe,NULL) ;

	u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;

	return rs ;
}
/* end subroutine (process) */



/* LOCAL SUBROUTINES */



static void sighand_int(sn)
int	sn ;
{


	if_int = TRUE ;
}
/* end subroutine (sighand_int) */



