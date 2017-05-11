/* dialpass */

/* subroutine to dial over to a UNIX® domain socket in stream mode */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1999-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	= Dial UNIX® Pass File (dialpass)

        This subroutine will dial out to an UNIX® domain socket stream address.

	Synopsis:

	int dialpass(fname,timeout,opts)
	const char	fname[] ;
	int		timeout ;
	int		opts ;

	Arguments:

	fname		path to UNIX® domain socket to dial to
	timeout		timeout ('>=0' mean use one, '-1' means don't)
	opts		any dial opts

	Returns:

	>=0		file descriptor
	<0		error in dialing


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/poll.h>
#include	<sys/socket.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		(8 * 1024)
#endif

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

static int	waitready(int,int) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED2 */
int dialpass(fname,timeout,opts)
const char	fname[] ;
int		timeout ;
int		opts ;
{
	struct ustat	sb ;

	int	rs ;
	int	oflags ;
	int	pipes[2] ;
	int	fd_pass ;
	int	fd = 0 ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("dialpass: fname=%s\n",fname) ;
	debugprintf("dialpass: timeout=%d\n",timeout) ;
#endif

/* try to open the pass file */

	oflags = (O_WRONLY | O_NDELAY) ;
	rs = u_open(fname,oflags,0666) ;
	fd_pass = rs ;
	if (rs < 0)
	    goto badpassopen ;

/* is the file a plausible type to pass an FD over? */

	rs = u_fstat(fd_pass,&sb) ;
	if (rs < 0)
	    goto badpassstat ;

	if ((! S_ISFIFO(sb.st_mode)) && (! S_ISCHR(sb.st_mode))) {
	    rs = SR_INVALID ;
	    goto badpassstat ;
	}

#if	CF_DEBUGS
	debugprintf("dialpass: plausible file type for passing\n") ;
#endif

/* wait until (while timing) the file is ready for the pass */

	if (timeout >= 0) {

#if	CF_DEBUGS
	    debugprintf("dialpass: waitready() to=%d\n",timeout) ;
#endif

	    rs = waitready(fd_pass,timeout) ;

#if	CF_DEBUGS
	    debugprintf("dialpass: waitready() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto badtimeout ;

	} /* end if (timeout) */

/* OK, make a pipe to pass down */

	rs = u_pipe(pipes) ;
	fd = pipes[0] ;
	if (rs < 0)
	    goto badpipe ;

/* do the actual pass operation */

	rs = u_ioctl(fd_pass,I_SENDFD,pipes[1]) ;

#if	CF_DEBUGS
	debugprintf("dialpass: u_ioctl() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    u_close(fd) ;

/* clean up */

	u_close(pipes[1]) ;

badpipe:
badtimeout:
badpassstat:
	u_close(fd_pass) ;

/* get out */
badpassopen:

#if	CF_DEBUGS
	debugprintf("dialpass: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialpass) */


/* local subroutines */


static int waitready(fd,timeout)
int		fd ;
int		timeout ;
{
	struct pollfd	polls[2] ;

	time_t	ti_timeout ;
	time_t	daytime = time(NULL) ;

	int	rs = SR_OK ;
	int	size ;
	int	pollto ;
	int	f = FALSE ;


#if	CF_DEBUGS
	debugprintf("dialpass/waitready: timeout=%d\n",timeout) ;
	debugprintf("dialpass/waitready: fd=%d\n",fd) ;
#endif

	if (timeout < 0)
	    goto ret0 ;

	size = 2 * sizeof(struct pollfd) ;
	memset(polls,0,size) ;

	polls[0].fd = fd ;
	polls[0].events = (POLLOUT | POLLWRBAND) ;
	polls[1].fd = -1 ;
	polls[1].events = 0 ;

	ti_timeout = daytime + timeout ;

/* CONSTCOND */

	while (rs >= 0) {

	    pollto = MIN(timeout,5) * POLLMULT ;

#if	CF_DEBUGS
	    debugprintf("dialpass/waitready: pollto=%d\n",pollto) ;
#endif

	    rs = u_poll(polls,1,pollto) ;

#if	CF_DEBUGS
	    debugprintf("dialpass/waitready: u_poll() rs=%d\n",rs) ;
#endif

	    if (rs > 0) {

		int	re = polls[0].revents ;

#if	CF_DEBUGS
	        {
	            char	buf[BUFLEN + 1] ;
	            debugprintf("dialpass/waitready: revents=%s\n",
	                d_reventstr(re,buf,BUFLEN)) ;
	        }
#endif /* CF_DEBUGS */

	        if (re & POLLHUP) {
	            rs = SR_HANGUP ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLNVAL) {
	            rs = SR_BADF ;
	        } else if ((re & POLLOUT) || (re & POLLWRBAND)) {
		    f = TRUE ;
		}

	    } else if (rs == SR_INTR)
		rs = SR_OK ;

	    if ((rs >= 0) && (! f)) {
	        daytime = time(NULL) ;
		if (daytime >= ti_timeout)
	            rs = SR_TIMEDOUT ;
	    }

	    if ((rs >= 0) && f)
		break ;

	} /* end while */

ret0:

#if	CF_DEBUGS
	debugprintf("dialpass/waitready: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (waitready) */


