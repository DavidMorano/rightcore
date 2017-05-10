/* watch */

/* watch (listen on) the specified socket */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1991-09-10, David A­D­ Morano

	This subroutine was adopted from the DWD program.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine is responsible for listening on the given socket
	and spawning off a program to handle any incoming connection.

	Arguments:

	pip	global data pointer
	s	socket to listen on
	elp	vector list of exported variables

	Returns:

	OK	doesn't really matter in the current implementation


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/time.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<limits.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<poll.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#define	W_OPTIONS	(WNOHANG)

#define	POLLTIMEOUT	1
#define	MAINTTIME	(2 * 60)


/* external subroutines */

extern int	handle(struct proginfo *,int,VECSTR *) ;
extern int	checklockfile(struct proginfo *,bfile *,
			const char *,const char *,time_t,pid_t) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*timestr_logz(time_t,char *) ;

#if	CF_DEBUG
extern char	*d_reventstr() ;
#endif


/* external variables */


/* forward references */

static void	int_all(int) ;


/* local global variables */

int	f_exit = FALSE ;


/* local structures */


/* local variables */


/* exported subroutines */


int watch(pip,s,elp)
struct proginfo	*pip ;
int		s ;
VECSTR		*elp ;
{
	struct pollfd	fds[2] ;

	struct sigaction	sigs ;

	struct sockaddr_in	from ;

	struct ustat		sb ;

	pid_t		pid ;

	sigset_t	signalmask ;

	time_t		t_lockcheck = 1 ;
	time_t		t_pidcheck = 1 ;

	int	rs ;
	int	i, len ;
	int	polltimeout = POLLTIMEOUT * 1000 ;
	int	child_stat ;
	int	nfds ;
	int	re ;
	int	fromlen ;
	int	ns ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUG
	char	buf[BUFLEN + 1] ;
#endif


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: entered w/ socket on FD %d\n",s) ;
#endif

/* we want to receive the new socket (from 'accept') above these guys */

	for (i = 0 ; i < 3 ; i += 1) {
		if (u_fstat(i,&sb) < 0) 
			u_open(NULLFNAME,O_RDONLY,0666) ;
	}

	uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGTERM,&sigs,NULL) ;

	uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGHUP,&sigs,NULL) ;

	uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGINT,&sigs,NULL) ;

/* let's go! */

	nfds = 0 ;
	fds[nfds].fd = s ;
	fds[nfds].events = POLLIN | POLLPRI ;
	fds[nfds].revents = 0 ;
	nfds += 1 ;

/* top of loop */
top:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: about to poll\n") ;
#endif

	rs = u_poll(fds,nfds,polltimeout) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: back from poll w/ rs=%d\n",rs) ;
#endif

	if (rs > 0) {

	    re = fds[0].revents ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("watch: back from poll, re=%s\n",
	            d_reventstr(re,buf,BUFLEN)) ;
#endif

	    if ((re & POLLIN) || (re & POLLPRI)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("watch: got a poll in, normal=%d priority=%d\n",
	    		(re & POLLIN) ? 1 : 0,(re & POLLPRI) ? 1 : 0) ;
#endif

	        fromlen = sizeof(struct sockaddr) ;
	        ns = u_accept(s,&from,&fromlen) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("watch: we accepted a call, rs=%d\n",ns) ;
#endif

	        if (ns < 0) {

			rs = ns ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf(
			"watch: bad return from accept, rs=%d\n",
			ns) ;
#endif

	            goto baderr ;
	        }

/* let's fork the processing subroutine and get on with our lives! */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("watch: about to fork, passing ns=%d\n",ns) ;
#endif

	        rs = uc_fork() ;
		pid = rs ;
	        if ((rs >= 0) && (pid == 0)) {

/* we are now the CHILD !! */

	            for (i = 0 ; i < 3 ; i += 1) 
			u_close(i) ;

	            u_close(s) ;

	            u_dup(ns) ;

	            u_dup(ns) ;

		    u_open(NULLFNAME,O_RDONLY,0666) ;

	            rs = handle(pip,ns,elp) ;

	            uc_exit(EX_NOEXEC) ;

	        } /* end if */

	        if (rs < 0)
	            logfile_printf(&pip->lh,
			"could not fork (%d)\n", rs) ;

	        u_close(ns) ;

	    } else if (re & POLLHUP) {

		rs = SR_HANGUP ;
	        goto badhup ;

	    } else if (re & POLLERR) {

		rs = SR_IO ;
	        goto baderr ;

	    } else if (re & POLLNVAL) {

		rs = SR_INVALID ;
	        goto baderr ;

	    }

	} /* end if */

	if (rs == SR_INTR)
	    rs = SR_OK ;

/* are any child processes finished? */

	if (rs >= 0) {
	    u_waitpid(-1,&child_stat,W_OPTIONS) ;
	    pip->daytime = time(NULL) ;
	}

/* maintenance the lock file */

	if ((rs >= 0) && (pip->lockfp != NULL) && 
	    ((pip->daytime - t_lockcheck) >= (MAINTTIME - 1))) {

	    rs = checklockfile(pip,pip->lockfp,pip->lockfname,
			BANNER,pip->daytime,pip->pid) ;
	    if (rs != 0) {

	        logfile_printf(&pip->lh,
	            "%s another program has my lock file, other PID=%d\n",
	            timestr_logz(pip->daytime,timebuf),
	            rs) ;

	        goto badlockfile ;
	    }

	    t_lockcheck = pip->daytime ;

	} /* end if (maintaining the lock file) */

/* maintenance the PID mutex lock file */

	if ((rs >= 0) && (pip->pidfp != NULL) && 
	    ((pip->daytime - t_pidcheck) >= (MAINTTIME - 1))) {

	    if ((rs = checklockfile(pip,pip->pidfp,pip->pidfname,
			BANNER,pip->daytime,pip->pid)) != 0) {

	        logfile_printf(&pip->lh,
	            "%s another program has my PID file, other PID=%d\n",
	            timestr_logz(pip->daytime,timebuf),
	            rs) ;

	        goto badpidfile ;
	    }

	    t_pidcheck = pip->daytime ;

	} /* end if (maintaining the PID mutex file) */

/* go back to the top of the loop */

	if ((rs >= 0) && (! f_exit))
		goto top ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("watch: returning !\n") ;
#endif

	pip->daytime = time(NULL) ;

	logfile_printf(&pip->lh,"%s daemon exiting (%d)\n",
	    timestr_logz(pip->daytime,timebuf),rs) ;

ret0:
	return rs ;

/* we got killed , sigh! */
killed:
badlockfile:
badpidfile:
	goto ret0 ;

/* bad stuff comes here */
baderr:
badhup:
	goto ret0 ;
}
/* end subroutine (watch) */


static void int_all(sn)
int	sn ;
{


	f_exit = TRUE ;
}
/* end subroutine (int_term) */



