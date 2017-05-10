/* transfer */

/* subroutine to handle the transfer phase of a connection */


#define	CF_DEBUG	1


/* revision history:

	- David A.D. Morano, 97/06/28
	This subroutine was derived from a previous version of the
	REX program.  This really just represents a repartitioning
	of the REX program in general.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	transfer()


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<string.h>
#include	<time.h>
#include	<stropts.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<vecelem.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"netfile.h"


/* local defines */

#define		NFDS		5



/* external subroutines */

extern int	getnodedomain() ;
extern int	authfile() ;
extern int	cfdec() ;
extern int	rex_rexec(), rex_rcmd() ;
extern int	hostequiv() ;

extern char	*putheap() ;
extern char	*strshrink() ;
extern char	*strbasename() ;
extern char	*timestr_log() ;
extern char	*d_reventstr() ;


/* forward subroutines */


/* external variables */

extern int	errno ;


/* global variables (localled declared) */

struct global	g ;

struct userinfo	u ;




int transfer()
{
	struct ustat	sb, isb, osb, esb ;

	struct pollfd	fds[NFDS] ;

	struct vecelem	ne, tmp ;

	struct netrc	*mp ;

	time_t		t_pollsanity, t_sanity ;

	int	i, j ;
	int	rs, len, l ;
	int	f_noinput = FALSE ;
	int	f_initlist = FALSE ;
	int	f_in0, f_out1, f_out2, f_in3, f_out3, f_in4 ;
	int	f_final0, f_final1, f_final2, f_final3, f_final4 ;
	int	ifd = 0, ofd = 1, efd = 2 ;
	int	f_eof0, f_eof3, f_eof4 ;
	int	f_exit ;
	int	f_euid ;
	int	f_x = FALSE ;
	int	f_d = FALSE ;
	int	f_envfile, f_rxport ;
	int	rfd, rfd2 ;
	int	pollinput = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;
	int	polloutput = POLLWRNORM | POLLWRBAND ;
	int	sanityfailures = 0 ;

	char	buf[BUFLEN + 1], *bp ;
	char	*cp, *cp1, *cp2 ;
	char	*hostname = NULL, *chostname, *cnodename, *cdomainname = NULL ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("transfer: entered\n") ;
#endif

/* setup the 'poll(2)' structures for use later */

	f_in0 = FALSE ;
	f_out1 = FALSE ;
	f_out2 = FALSE ;
	f_in3 = FALSE ;
	f_out3 = FALSE ;
	f_in4 = FALSE ;
	f_eof0 = f_eof3 = f_eof4 = FALSE ;
	f_final0 = f_final1 = f_final2 = f_final3 = f_final4 = FALSE ;

/* standard input */

	fds[0].fd = -1 ;
	if (! f_noinput) {

	    fds[0].fd = ifd ;
	    fds[0].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;

	} else {

	    f_eof0 = TRUE ;
	    if ((fstat(rfd,&sb) >= 0) && 
	        (S_ISSOCK(sb.st_mode) || S_ISCHR(sb.st_mode)))
	        shutdown(rfd,1) ;

	    else
	        write(rfd,buf,0) ;

	}

/* standard output */

	fds[1].fd = -1 ;
	if ((rs = fstat(ofd,&osb)) >= 0) {

	    fds[1].fd = ofd ;
	    fds[1].events = POLLWRNORM | POLLWRBAND ;

	}

/* standard error */

	fds[2].fd = -1 ;
	if (fstat(2,&esb) >= 0) {

	    bflush(efp) ;

	    fds[2].fd = 2 ;
	    fds[2].events = POLLWRNORM | POLLWRBAND ;

	}

/* remote socket */

	fds[3].fd = rfd ;
	fds[3].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;
	fds[3].events |= (fds[3].events | POLLWRNORM | POLLWRBAND) ;

/* remote error socket */

	fds[4].fd = rfd2 ;
	fds[4].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;


/* do it */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("nain: we got connected\n") ;
#endif

	bflush(efp) ;

/* what about sanity checking */

	if (g.f.sanity) {

	    t_pollsanity = 0 ;
	    t_sanity = 1 ;
	    sanityfailures = 0 ;

	}


/* do the copy data function */

	f_exit = FALSE ;
	while (! f_exit) {

	    if (poll(fds,NFDS,POLLTIME) < 0) rs = (- errno) ;

#if	CF_DEBUG
	    if (g.debuglevel > 2)
	        debugprintf("transfer: back from POLL w/ rs=%d\n",
	            rs) ;
#endif

	    if (rs < 0) {

	        if (errno == EAGAIN) {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("transfer: back from POLL w/ EAGAIN\n") ;
#endif

	            sleep(2) ;

	            continue ;

	        } else if (errno == EINTR) {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("transfer: back from POLL w/ EINTR\n") ;
#endif

	            continue ;

	        } else {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("transfer: back from POLL w/ BADPOLL\n") ;
#endif

	            goto badpoll ;
	        }

	    } /* end if (poll got an error) */

#if	CF_DEBUG
	    if (g.debuglevel > 2) {

	        for (i = 0 ; i < NFDS ; i += 1) {

	            debugprintf("transfer: fds%d %s\n",i,
	                d_reventstr(fds[i].revents,buf,BUFLEN)) ;

	        }
	    }
#endif

/* check the actual low level events */

	    if (fds[0].revents & pollinput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("transfer: IN0\n") ;
#endif

	        f_in0 = TRUE ;
	        fds[0].events &= (~ pollinput) ;

	    }

	    if (fds[0].revents & POLLHUP) {

	        f_in0 = TRUE ;
	        f_final0 = TRUE ;
	        fds[0].events &= (~ pollinput) ;
	        fds[0].fd = -1 ;

	    }

	    if (fds[1].revents & polloutput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("transfer: OUT1\n") ;
#endif

	        f_out1 = TRUE ;
	        fds[1].events &= (~ polloutput) ;

	    }

	    if (fds[2].revents & polloutput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("transfer: OUT2\n") ;
#endif

	        f_out2 = TRUE ;
	        fds[2].events &= (~ polloutput) ;

	    }

	    if (fds[3].revents & pollinput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("transfer: IN3\n") ;
#endif

	        f_in3 = TRUE ;
	        fds[3].events &= (~ pollinput) ;

	    }

	    if ((fds[3].revents & POLLHUP) ||
	        (fds[3].revents & POLLERR) ||
	        (fds[3].revents & POLLNVAL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 2) {

	            if (fds[3].revents & POLLHUP)
	                debugprintf("transfer: 3 POLLHUP\n") ;

	            if (fds[3].revents & POLLERR)
	                debugprintf("transfer: 3 POLLERR\n") ;

	            if (fds[3].revents & POLLNVAL)
	                debugprintf("transfer: 3 POLLNVAL\n") ;

	        }
#endif

	        f_in3 = TRUE ;
	        f_final3 = TRUE ;
	        fds[3].fd = -1 ;

	    }

	    if (fds[3].revents & polloutput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("transfer: OUT3\n") ;
#endif

	        f_out3 = TRUE ;
	        fds[3].events &= (~ polloutput) ;

	    }

	    if (fds[4].revents & pollinput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("transfer: IN4\n") ;
#endif

	        f_in4 = TRUE ;
	        fds[4].events &= (~ pollinput) ;

	    }

	    if (fds[4].revents & POLLHUP) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("transfer: HUP4\n") ;
#endif

	        f_final4 = TRUE ;
	        f_in4 = TRUE ;
	        fds[4].fd = -1 ;

	    }

/* what did we not look at */

#if	! CF_DEBUG
	    if ((! f_in0) && (! f_out1) && (! f_out2) && (! f_in3)
	        && (! f_in4)) {

	        if (g.debuglevel > 2) {

	            debugprintf("transfer: fd0=%08X\n",fds[0].revents) ;

	            debugprintf("transfer: fd1=%08X\n",fds[1].revents) ;

	            debugprintf("transfer: fd2=%08X\n",fds[2].revents) ;

	            debugprintf("transfer: fd3=%08X\n",fds[3].revents) ;

	            debugprintf("transfer: fd4=%08X\n",fds[4].revents) ;

	        }

	        break ;
	    }
#else
	    if (g.debuglevel > 2) {

	        debugprintf("transfer: fd0=%08X\n",fds[0].revents) ;

	        debugprintf("transfer: fd1=%08X\n",fds[1].revents) ;

	        debugprintf("transfer: fd2=%08X\n",fds[2].revents) ;

	        debugprintf("transfer: fd3=%08X\n",fds[3].revents) ;

	        debugprintf("transfer: fd4=%08X\n",fds[4].revents) ;

	    }
#endif

/* now we are ready to check for the events that we really want */

/* output from remote command to our standard output */

	    if (f_in3 && f_out1) {

	        len = read(rfd,buf,BUFLEN) ;

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("transfer: IN3 -> OUT1 len=%d errno=%d\n",
	                len,errno) ;
#endif
	        if (len > 0) {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("transfer: IN3 -> OUT1 \"%W\"\n",buf,len) ;
#endif

	            writen(fds[1].fd,buf,len) ;

	        } else {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("transfer: IN3 EOF\n") ;
#endif
	            f_eof3 = TRUE ;
	            if (f_eof0) {

	                fds[3].fd = -1 ;

	            } else {

	                fds[3].events &= (~ pollinput) ;

	            }

	        }

	        if (! f_final3) {

	            f_in3 = FALSE ;
	            fds[3].events |= pollinput ;

	        }

	        f_out1 = FALSE ;
	        fds[1].events |= polloutput ;
	    }

/* input from our standard input out to the remote command */

	    if (f_in0 && f_out3) {

	        len = read(ifd,buf,BUFLEN) ;

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("transfer: IN0 -> OUT3  len=%d\n",len) ;
#endif

	        if (len > 0)
	            writen(fds[3].fd,buf,len) ;

	        else {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("transfer: IN0 EOF\n") ;
#endif
	            f_eof0 = TRUE ;
	            f_in0 = FALSE ;
	            fds[0].fd = -1 ;
#if	CF_DEBUG
	            if (g.debuglevel > 2) {
	                rs = fstat(rfd,&sb) ;

	                debugprintf("transfer: RFD stat rs=%d mode=%08X\n",
	                    rs,sb.st_mode) ;
	            }
#endif

	            if ((fstat(rfd,&sb) >= 0) && 
	                (S_ISSOCK(sb.st_mode) || S_ISCHR(sb.st_mode))) {

	                shutdown(rfd,1) ;

#if	CF_DEBUG
	                if (g.debuglevel > 2)
	                    debugprintf("transfer: we shutdown the remote FD\n") ;
#endif
	            } else
	                write(rfd,buf,0) ;
	        }

	        if (! f_final0) {

	            f_in0 = FALSE ;
	            fds[0].events |= pollinput ;
	        }

	        f_out3 = FALSE ;
	        fds[3].events |= polloutput ;
	    }

/* output from remote command error channel to our standard error output */

	    if (f_in4 && f_out2) {

	        len = read(rfd2,buf,BUFLEN) ;

	        if (len > 0)
	            writen(fds[2].fd,buf,len) ;

	        else {

	            f_eof4 = TRUE ;

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("transfer: IN4 EOF\n") ;
#endif
	        }

	        if (! f_final4) {

	            f_in4 = FALSE ;
	            fds[4].events |= pollinput ;

	        }

	        f_out2 = FALSE ;
	        fds[2].events |= polloutput ;
	    }

	    if (f_eof3 && f_eof4) break ;

/* miscellaneous functions */

	    if (g.f.sanity) {

	        time(&g.daytime) ;

	        if ((g.daytime - t_pollsanity) > 
	            (g.keeptime / SANITYFAILURES)) {

	            t_pollsanity = g.daytime ;
	            if (ping(hostname) >= 0) {

	                sanityfailures = 0 ;
	                t_sanity = g.daytime ;

	            } else
	                sanityfailures += 1 ;

	            if (((g.daytime - t_sanity) > g.keeptime) &&
	                (sanityfailures >= SANITYFAILURES) &&
	                (ping(hostname) < 0)) goto badping ;

	        } /* end if (sanity poll) */

	    } /* end if (sanity check) */

	} /* end while (transferring data) */

	close(rfd) ;

	close(rfd2) ;

#ifdef	COMMENT
	if (g.f.verbose) bprintf(efp,
	    "%s: remote command completed (rs %d)\n",
	    g.progname,rs) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("transfer: finishing up\n") ;
#endif

/* finish up */
done:
	rs = OK ;
	close(ofd) ;

	logfile_close(&g.lh) ;

/* take the early return here */
earlyret:
	bclose(efp) ;

	return rs ;

/* bad returns come here */
badret:

#if	CF_DEBUG
	bflush(efp) ;
#endif

	close(ofd) ;

	bclose(efp) ;

	return BAD ;


hostdown:
	logfile_printf(&g.lh,"host is down\n") ;

	if (! g.f.quiet)
	    bprintf(efp,"%s: host is down (or unreachable)\n",
	        g.progname) ;

	goto badret ;

badpoll:
	bprintf(efp,"%s: bad error return from a poll (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

/* we failed the "ping"ing protocol */
badping:
	time(&g.daytime) ;

	l = strlen(hostname) ;

	if (((cp = strchr(hostname,'.')) != NULL) &&
	    (strcasecmp(cp + 1,u.domainname) == 0))
	    l = cp - hostname ;

	logfile_printf(&g.lh,
	    "%s host \"%W\" went down\n",
	    timestr_log(g.daytime,buf),
	    hostname,l) ;

	if (! g.f.quiet)
	    bprintf(efp,"%s: %s host \"%W\" went down\n",
	        g.progname,
	        timestr_log(g.daytime,buf),
	        hostname,l) ;

	goto badret ;

}
/* end subroutine (transfer) */



