/* process */

/* do the "comsat" part of this thing ! */


#define	CF_DEBUGS	1
#define	CF_DEBUG	1


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine reads the message and logs it.


******************************************************************************/


#include	<envstandards.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN	MAXPATHLEN
#endif

#ifndef	TO
#define	TO	20
#endif


/* external subroutines */

extern int	listenudp(const char *,const char *,int) ;


/* forward references */

static int	isprintable(const char *,int) ;


/* local variables data */

static int	f_exit = FALSE ;


/* exported subroutines */


int process(pip,hostname,portspec,to)
struct proginfo	*pip ;
const char	hostname[], portspec[] ;
int		to ;
{
	struct fd_set	fds ;

	SOCKADDRESS	from ;

	time_t	marktime, daytime ;

	int	rs = SR_OK ;
	int	i, len ;
	int	c = 0 ;
	int	fromlen ;
	int	fd_stdin, fd_stdout ;
	int	ifd, ofd ;
	int	f_daemon = pip->f.daemon ;
	int	f_once ;

	char	netbuf[BUFLEN + 1] ;
	char	outbuf[BUFLEN + 1] ;
	char	timebuf1[TIMEBUFLEN + 1] ;
	char	timebuf2[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("process: hostname=%s portspec=%s to=%d f_daemon=%u\n",
		hostname,portspec,to,f_daemon) ;
#endif

	fd_stdin = FD_STDIN ;
	fd_stdout = FD_STDOUT ;

	    ifd = fd_stdin ;
	    ofd = fd_stdout ;
	    if (f_daemon) {

	        rs = listenudp(hostname,portspec,0) ;

	        if (rs < 0)
	            goto badlisten ;

#if	CF_DEBUGS
	        debugprintf("process: listenudp() rs=%d\n",rs) ;
#endif

	        ifd = rs ;
	        ofd = rs ;

	    } /* end if (daemon mode) */

		marktime = time(NULL) ;

	f_once = TRUE ;
	    while ((f_once || f_daemon) && (! f_exit)) {

		f_once = FALSE ;

	        fromlen = sizeof(SOCKADDRESS) ;
	        rs = uc_recvfrome(ifd,netbuf,BUFLEN,0,&from,&fromlen,TO,0) ;
		len = rs ;

#if	CF_DEBUGS
	        debugprintf("process: u_recvfrom() rs=%d\n",rs) ;
		if (len >= 0)
	        debugprintf("process: buf=>%t<\n", netbuf,len) ;
#endif /* CF_DEBUGS */

		daytime = time(NULL) ;

	        if (len >= 0) {

#if	CF_DEBUGS
	            debugprintf("process: len=%d\n",len) ;
#endif

		c += 1 ;

#ifdef	COMMENT
	            rs = u_sendto(ofd, netbuf,
	                len, 0, &from, fromlen) ;
#if	CF_DEBUGS
	            debugprintf("process: u_sendto() rs=%d\n",rs) ;
#endif

#endif /* COMMENT */

		if (pip->f.log && isprintable(netbuf,len)) {

			logfile_printf(&pip->lh,"%t\n",
				netbuf,len) ;

			logfile_flush(&pip->lh) ;

		}

			marktime = daytime ;

	        } /* end if (non-zero length) */

	            if ((rs < 0) && (rs != SR_TIMEDOUT))
	                break ;

#if	CF_DEBUGS
	            debugprintf("process: marktime=%s daytime=%s\n",
			timestr_log(marktime,timebuf1),
			timestr_log(daytime,timebuf2)) ;
#endif

		if ((daytime - marktime) > to)
			break ;

	    } /* end while (daemon mode) */

	if (rs == SR_TIMEDOUT)
		rs = SR_OK ;

#if	CF_DEBUGS
	            debugprintf("process: out while, f_daemon=%u f_exit=%u\n",
			f_daemon,f_exit) ;
#endif

		u_close(ifd) ;

ret2:
	close(fd_stdin) ;

ret1:
	close(fd_stdout) ;

ret0:
badlisten:
	return (rs >= 0) ? c : rs ;

}
/* end subroutine (process) */



/* LOCAL SUBROUTINES */



#ifdef	COMMENT

static int int_exit(sn)
int	sn ;
{


	f_exit = TRUE ;
}

#endif /* COMMENT */


static int isprintable(cchar *buf,int buflen)
{
	int		i ;
	int		f = TRUE ;

	if (buflen < 0) buflen = strlen(buf) ;

	for (i = 0 ; i < buflen ; i += 1) {
	    const int	ch = MKCHAR(buf[i]) ;
	    f = isprintlatin(ch) ;
	    if (!f) break ;
	} /* end for */

	return f ;
}
/* end subroutine (isprintable) */


