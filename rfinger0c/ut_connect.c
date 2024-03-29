/* ut_connect */


#define	CF_DEBUGS	0
#define	CF_ISCONN	0		/* change meaning of SR_ISCONN? */
#define	CF_BADSOLARIS	0		/* bad Solaris? */


/* revision history:

	= 1989-05-04, David Morano

	I needed this subroutine (and the whole 'ut' set of
	subroutines) becauase one cannot connect to a 'ticotsord'
	transport with a UNIX domain socket.  This may have been
	possible in the past but must have gone away sometime around
	1990 or so, maybe.  It probably went away when some smart
	person working on UNIX System V Release 4 networking decided to
	make their sockets work more like the ones in BSD!


	= 1992-02-14, David Morano

	I updated this subroutine this some stuff from the 'u_connect()'
	subroutine that has to do with a connect attempt after one was
	previously done but in which the previous one was done with the
	file descriptor in non-blocking mode and the latter one done
	with the file descriptor in blocking mode.  Does that ever
	really happen (anymore)?  I guess that it did at one time!
	With all of the changes to the way sockets work now-a-days in
	different OSes, who can know what happens when (really)!!


*/


/******************************************************************************

	This subroutine does what the socket counter part 'u_connect()'
	does.  It is pretty much similar but the semantics of the call
	are not entirely clear.  Oh, this subroutine is also using the
	original TLI API rather than the X/Open (read European!) XTI
	API.  Who knows what XTI looks like at this point!

	Finally, I do not know if Solaris is bad on this TLI crap or
	not!  I am assumming that it *might* be OK but the code
	is in there if you want to turn it on with the compile-time
	switch 'CF_BADSOLARIS' above.  Enjoy!


******************************************************************************/


#define	LIBUT_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<tiuser.h>
#include	<errno.h>

#include	<vsystem.h>

#include	"localmisc.h"



/* local defines */

#define	TO_NOMEM	60			/* seconds */
#define	TO_NOSR		(5 * 60)		/* seconds */
#define	TO_CONNECT	(5 * 60)		/* seconds */
#define	POLLTIMEOUT	(1 * 1000)		/* milliseconds */

#if	CF_DEBUGS
#define	BUFLEN		100
#endif



/* local structures */

struct connect_flags {
	uint	checkblock : 1 ;
	uint	nonblock : 1 ;
} ;


/* forward references */

static int	checkconnect() ;
static int	lconnect() ;






int ut_connect(fd,sndcall,rcvcall)
int		fd ;
struct t_call	*sndcall, *rcvcall ;
{
	struct connect_flags	f = { 
	    FALSE, FALSE 	
	} ;

	int	rs, rs2 ;
	int	to_nosr = 0 ;

#if	CF_DEBUGS
	char	buf[BUFLEN + 1] ;
#endif

#if	CF_DEBUGS
	debugprintf("ut_connect: entered, FD=%d\n",s) ;
#endif

again:
	rs = lconnect(fd,sndcall,rcvcall) ;

	if (rs < 0) {

	    switch (rs) {

	    case SR_NOSR:
	        to_nosr += 1 ;
	        if (to_nosr > TO_NOSR) break ;

	        sleep(1) ;

	        goto again ;

	    case SR_INTR:

#if	CF_DEBUGS
	        debugprintf("ut_connect: interrupted\n") ;
#endif

	        goto again ;

/* handle the case when it is still outstanding if we can */
	    case SR_ALREADY:
	    case SR_PROTO:
	        rs2 = checkconnect(&f,fd) ;

	        if (rs2 < 0) {

	            rs = rs2 ;
	            goto err2 ;
	        }

	        if (! f.nonblock) {

	            struct pollfd	fds[2] ;

	            int	nfds ;
	            int	looptimeout = (TO_CONNECT * 1000) ;


	            nfds = 0 ;
	            fds[nfds].fd = fd ;
	            fds[nfds].events = POLLOUT ;
	            fds[nfds].revents = 0 ;
	            nfds += 1 ;

	            while (looptimeout > 0) {

#if	CF_DEBUGS
	                debugprintf("ut_connect: about to poll\n") ;
#endif

	                rs = u_poll(fds,nfds,POLLTIMEOUT) ;

#if	CF_DEBUGS
	                debugprintf("ut_connect: poll back, rs=%d\n",rs) ;
#endif

	                if (rs > 0) {

	                    int	re ;


	                    re = fds[0].revents ;

#if	CF_DEBUGS
	                    debugprintf("ut_connect: back from poll, re=%s\n",
	                        d_reventstr(re,buf,BUFLEN)) ;
	                    debugprintf("ut_connect: take action on results\n") ;
#endif /* CF_DEBUGS */

	                    rs = SR_OK ;
	                    if (re & POLLOUT) {

#if	CF_DEBUGS
	                        debugprintf("ut_connect: goto the pollout\n") ;
#endif

#if	CF_BADSOLARIS
	                        rs = lconnect(fd,sndcall,rcvcall) ;

#if	CF_DEBUGS
	                        debugprintf("ut_connect: connect 2 rs=%d\n",rs) ;
#endif

/* is it connected successfully yet? */

	                        if (rs == SR_ISCONN)
	                            rs = SR_OK ;
#endif /* CF_BADSOLARIS */

	                        if (rs >= 0)
	                            break ;

#if	CF_BADSOLARIS

/* is it still in progress? */

	                        if (rs == SR_ALREADY)
	                            rs = SR_OK ;
#endif /* CF_BADSOLARIS */

	                    } else if (re & POLLHUP)
	                        rs = SR_HANGUP ;

	                    else if (re & POLLERR)
	                        rs = SR_POLLERR ;

	                    else if (re & POLLNVAL)
	                        rs = SR_NOTOPEN ;

	                    if (rs < 0)
	                        break ;

	                } /* end if */

	                looptimeout -= POLLTIMEOUT ;

#if	CF_DEBUGS
	                debugprintf("ut_connect: bottom while rs=%d lto=%d\n",
	                    rs,looptimeout) ;
#endif

	            } /* end while */

#if	CF_DEBUGS
	            debugprintf("ut_connect: out loop, rs=%d looptimeout=%d\n",
	                rs,looptimeout) ;
#endif

	            if (looptimeout <= 0)
	                rs = SR_TIMEDOUT ;

	        } /* end if (waiting for connection to complete) */

	        break ;


#if	CF_ISCONN
	    case SR_ISCONN:
	        rs2 = checkconnect(&f,s) ;

	        if (rs2 < 0) {

	            rs = rs2 ;
	            goto baderr ;
	        }

	        if (! f.nonblock)
	            rs = SR_OK ;
#else
	    case SR_ISCONN:
		;
#endif /* CF_ISCONN */

	    } /* end switch */

	} /* end if (error) */

#if	CF_DEBUGS
	debugprintf("ut_connect: exiting, rs=%d\n",rs) ;
#endif

err2:
	return rs ;
}
/* end subroutine (ut_connect) */



/* LOCAL SUBROUTINES */



static int checkconnect(fp,fd)
struct connect_flags	*fp ;
int			fd ;
{
	int	rs = SR_OK ;


	if (! fp->checkblock) {

	    fp->checkblock = TRUE ;
	    if ((rs = u_fcntl(fd,F_GETFL,0)) < 0)
	        goto err2 ;

	    if (rs & O_NONBLOCK)
	        fp->nonblock = TRUE ;

	} /* end if (block check) */

err2:
	return rs ;
}
/* end subroutine (checkconnect) */


static int lconnect(fd,sndcall,rcvcall)
int		fd ;
struct t_call	*sndcall, *rcvcall ;
{
	int	rs ;


	rs = t_connect(fd,sndcall,rcvcall) ;

	if (rs < 0) {

	    switch (t_errno) {

	    case TACCES:
	        rs = SR_ACCES ;
	        break ;

	    case TBADADDR:
	        rs = SR_BADFMT ;
	        break ;

	    case TBADDATA:
	        rs = SR_TOOBIG ;
	        break ;

	    case TBADF:
	        rs = SR_NOTSOCK ;
	        break ;

	    case TBADOPT:
	        rs = SR_PROTOTYPE ;
	        break ;

	    case TBUFOVFLW:
	        rs = SR_OVERFLOW ;
	        break ;

	    case TLOOK:
	        rs = SR_BUSY ;
	        break ;

/* this is what is returned when connecting asynchronously */
	    case TNODATA:
	        rs = SR_OK ;
	        break ;

	    case TNOTSUPPORT:
	        rs = SR_OPNOTSUPP ;
	        break ;

	    case TOUTSTATE:
	        rs = SR_PROTO ;
	        break ;

	    case TSYSERR:
	        rs = (- errno) ;
	        break ;

	    }
	}

	return rs ;
}
/* end subroutine (lconnect) */



