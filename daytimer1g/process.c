/* daytimer */

/* day timer subroutine */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_POLL		1
#define	CF_ISPROC	0		/* use 'isproc()' ? */


/* revision history:

	= 1988-02-01, David A­D­ Morano

	This subroutine was originally written.


	= 1998-05-01, David A­D­ Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


	= 1998-12-01, David A­D­ Morano

	This subroutine has been updated to use the new Lock File
	Manager (lfm) and also the new display manager object for
	management of the display.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the subroutine which performs the active work for
	the 'daytime' program.


	Implementtion note:

	I do not exactly know why orphan-detection is so important but
	it is when things start to go badly !  Anyway, there is choice
	of using 'isproc()' or 'u_kill(2)' for detecting if our session
	leader is gone or not.  The 'isproc()' call tells us if any
	process whose PID is specified in that call is alive on the
	system or not.  The 'u_kill(2)' call allows us to only
	determine whether a process that we have the right to kill (!)
	is alive on the system.  Since this program will not generally
	be run by one user where it session leader-process is owned by
	another person, it is probably (?) safe to assume that our
	session leader is owned by the same owner as we are.  Is this a
	valid assumption ?


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<stropts.h>
#include	<signal.h>
#include	<poll.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<userinfo.h>
#include	<bfile.h>
#include	<lfm.h>
#include	<localmisc.h>

#include	"mailfiles.h"
#include	"config.h"
#include	"defs.h"
#include	"display.h"


/* local defines */


/* external subroutines */

extern int	isproc(pid_t) ;

extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* forward references */

static void	int_all(int) ;


/* local file-scope variables */

static int	if_all = FALSE ;	/* interrupt-flag for "all" */
static int	signal_num = 0 ;


/* exported subroutines */


int process(pip,up,fd_display,ti_offset,ti_refresh,f_statdisplay,mfp)
struct proginfo	*pip ;
struct userinfo	*up ;
int		fd_display ;
int		ti_offset, ti_refresh ;
int		f_statdisplay ;
MAILFILES	*mfp ;
{
	struct sigaction	sigs ;
	struct ustat	sb, ss, *sp = &ss ;
	struct pollfd	fds[2] ;
	sigset_t	signalmask ;
	DISPLAY		d ;

	pid_t	ppid ;
	pid_t	pgid ;

	time_t	daytime ;
	time_t	ti_access ;
	time_t	ti_prev, ti_cur ;
	time_t	ti_mail = 0L ;
	time_t	ti_checklocks = 0 ;

	long	elapsed ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	tic = 0 ;
	int	mail_tics = MAIL_TICS ;
	int	state = S_NOMAIL ;
	int	childstat ;
	int	nfds = 0 ;
	int	f_orphaned = FALSE ;
	int	f_newmail = FALSE ;
	int	f_dtime = FALSE ;
	int	f_mtime = FALSE ;
	int	f_displaying = TRUE ;
	int	f_blank = FALSE ;
	int	f_inputavail = FALSE ;
	int	f_update = FALSE ;
	int	f_badstat = FALSE ;
	int	f_baddisplay = FALSE ;
	int	f ;

	const char	*cp ;
	char	buf[BUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	timebuf1[TIMEBUFLEN + 1] ;
	char	timebuf2[TIMEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("daytimer: entered, refresh=%ld\n",
	        ti_refresh) ;
#endif

/* set the signals that we want to catch */

	uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGTERM,&sigs,NULL) ;

	uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGHUP,&sigs,NULL) ;

	uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGINT,&sigs,NULL) ;

/* get the access time of our display */

	rs = u_fstat(fd_display,&sb) ;

	ti_access = sb.st_atime ;
	if (rs < 0) {
	    f_badstat = TRUE ;
	    goto badstat ;
	}

/* initialize the POLL structure */

#if	CF_POLL
	nfds = 0 ;

	fds[nfds].fd = fd_display ;
	fds[nfds].events = POLLIN ;
	nfds += 1 ;

	fds[nfds].fd = -1 ;
	fds[nfds].events = POLLIN ;
#endif /* CF_POLL */

/* initialize the display object */

	rs = display_start(&d,fd_display,
	    f_statdisplay,ti_refresh,ti_offset) ;

	if (rs < 0)
	    goto baddisplay ;

/* main loop */

	daytime = time(NULL) ;

	ti_prev = daytime ;

	i = 0 ;
	while ((! f_orphaned) && (! if_all)) {

	    int	f_changed ;


/* get mail status */

	    if ((tic % mail_tics) == 0) {

	        f_changed = FALSE ;
	        if (mailfiles_check(mfp) > 0) {

	            f_changed = TRUE ;
	            ti_mail = daytime ;

	        }

	        if (f_changed) {

	            state = S_MAIL ;
	            mail_tics = 1 ;		/* fast update */
	            if (! f_newmail) {

	                f_newmail = TRUE ;

	            }

	        } else if (state = S_MAIL) {

	            state = S_MAILDONE ;
	            mail_tics = MAIL_TICS ;	/* slow update */
	            f_newmail = FALSE ;

	        } else {

	            state = S_NOMAIL ;
	            f_newmail = FALSE ;

	        }

	    } /* end if (getting mail file status) */

/* get terminal access time */

	    if (f_update) {

	        f_update = FALSE ;
	        ti_access = daytime ;

	    } else if (daytime > (ti_access + pip->to_blank)) {

	        rs = u_fstat(fd_display,&sb) ;

	        ti_access = MAX(sb.st_atime,ti_prev) ;
	        if (rs < 0) {
	            f_badstat = TRUE ;
	            break ;
	        }

	    } /* end if */

	    f_dtime = (daytime <= (ti_access + pip->to_blank)) ;

	    f_mtime = (daytime <= (ti_mail + pip->mailint)) ;

	    f_changed = FALSE ;
	    f_blank = FALSE ;
	    f = f_mtime || f_dtime ;
	    if (f_displaying && (! f)) {

	        ti_cur = ti_access ;
	        elapsed = (long) (MAX((ti_cur - ti_prev),0)) ;

	        f_displaying = FALSE ;
	        f_blank = TRUE ;
	        f_changed = TRUE ;
		if (pip->f.log) {

	        logfile_printf(&pip->lh,"%s idle from %s",
	            timestr_logz(daytime,timebuf),
	            timestr_logz(ti_access,timebuf1)) ;

	        logfile_printf(&pip->lh,"busy for %s",
	            timestr_elapsed(elapsed,timebuf2)) ;

		}

	        ti_prev = ti_cur ;

	    } else if ((! f_displaying) && f) {

	        ti_cur = daytime ;
	        elapsed = (long) (ti_cur - ti_prev) ;

	        f_changed = TRUE ;
	        f_displaying = TRUE ;
		if (pip->f.log) {

	        logfile_printf(&pip->lh,"%s active",
	            timestr_logz(daytime,timebuf)) ;

	        logfile_printf(&pip->lh,"idle for %s",
	            timestr_elapsed(elapsed,timebuf2)) ;

		}

	        ti_prev = ti_cur ;

	    }

	    if (f_changed && pip->f.log)
	        logfile_flush(&pip->lh) ;

	    if (state == S_MAILDONE)
	        state = S_NOMAIL ;

/* formulate the status display and write */

	    if (f_displaying || f_update || f_inputavail) {

		f_inputavail = FALSE ;
	        f_update = FALSE ;
	        rs = display_show(&d,daytime,f_newmail) ;

	        if (rs < 0) {
	            f_baddisplay = TRUE ;
	            break ;
	        }

	    } else if (f_blank) {

	        rs = display_blank(&d) ;

	        if (rs < 0) {
	            f_baddisplay = TRUE ;
	            break ;
	        }

	    } /* end if (blank requested) */

/* lock file checks */

	    if (pip->f.lockfile || pip->f.pidfile) {

	        if (daytime > (ti_checklocks + (pip->to_lock / 2))) {

			LFM_CHECK	ci ;


	            ti_checklocks = daytime ;

	            if (pip->f.lockfile) {

	                rs = lfm_check(&pip->lk,&ci,daytime) ;

	                if (rs == SR_LOCKLOST) {

	                    bprintf(pip->efp,
	                        "%s: detected a lock conflict PID=%u\n",
	                        pip->progname,(int) ci.pid) ;

			    if (pip->f.log)
	                    logfile_printf(&pip->lh,
	                        "detected a lock conflict PID=%u\n",
	                        (int) ci.pid) ;
			
	                    break ;
	                }

	            } /* end if (lockfile) */

	            if (pip->f.pidfile) {

	                rs = lfm_check(&pip->pidlock,&ci,daytime) ;

	                if (rs == SR_LOCKLOST) {

	                    bprintf(pip->efp,
	                        "%s: detected a PID lock conflict PID=%u\n",
	                        pip->progname,(int) ci.pid) ;

			    if (pip->f.log)
	                    logfile_printf(&pip->lh,
	                        "detected a PID lock conflict PID=%u\n",
	                        (int) ci.pid) ;
			
	                    break ;
	                }

	            } /* end if (pidfile) */

	        } /* end if (we have a time check interval */

	    } /* end if (checking lock files) */

#if	CF_POLL
	    if (! f_displaying) {

	        rs1 = u_poll(fds,1,(POLLMULT * pip->pollint)) ;

	        if (rs1 >= 0) {

	            if (fds[0].revents & POLLIN) {

	                if (! f_inputavail) {

	                    f_inputavail = TRUE ;
	                    f_update = TRUE ;

	                } else
	                    sleep(pip->pollint) ;

	            } else {

	                f_inputavail = FALSE ;

	            }
	        }

	    } else
	        sleep(pip->pollint) ;

#else /* CF_POLL */

	    sleep(pip->pollint) ;

#endif /* CF_POLL */

	    daytime = time(NULL) ;

/* see if we have been orphaned */

	    if (((tic % 5) == 0) && (! f_orphaned)) {

#if	CF_ISPROC

	        if (! isproc(pip->sid))
	            f_orphaned = TRUE ;

#else /* CF_ISPROC */

	        rs1 = u_kill(pip->sid,0) ;

	        if (rs1 == SR_SRCH)
	            f_orphaned = TRUE ;

#endif /* CF_ISPROC */

	    } /* end if (orphan check) */

/* logfile check */

	    if (pip->f.log) {

	    if ((tic % 10) == 0)
	        logfile_check(&pip->lh,daytime) ;

	    if ((tic % 100) == 0)
	        logfile_checksize(&pip->lh,pip->loglen) ;

	    }

/* bottom of loop processing */

	    tic = (tic + 1) & INT_MAX ;
	    i += 1 ;

	} /* end while (main loop) */

	if (f_baddisplay)
	    bprintf(pip->efp,"%s: bad display (%d)\n",
	        pip->progname,rs) ;

	display_finish(&d) ;

baddisplay:
badstat:
	if (f_badstat)
	    bprintf(pip->efp,
	        "%s: could not get status of terminal (%d)\n",
	        pip->progname,rs) ;

/* log termination status */

	if (f_orphaned || if_all) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("daytimer: orphaned or exit\n") ;
#endif

	    daytime = time(NULL) ;

	    i = bufprintf(buf,BUFLEN,"%s",
	        timestr_logz(daytime,timebuf)) ;

	    if (f_orphaned)
	        i += bufprintf((buf + i),BUFLEN," orphaned") ;

	    if (if_all) {

	        if (signal_num == SIGHUP) {
	            cp = " exiting (HUP)\n" ;

	        } else if (signal_num == SIGINT) {
	            cp = " exiting (INT)\n" ;

	        } else if (signal_num == SIGTERM)
	            cp = " exiting (TERM)\n" ;

	        i += bufprintf((buf + i),(BUFLEN - i),cp) ;

	    } /* end if (got an exit signal) */

#ifdef	COMMENT
	    i += bufprintf((buf + i),BUFLEN," rs=%d\n",rs) ;
#endif

	    if (pip->f.log)
	        logfile_printf(&pip->lh,buf) ;

	    if (f_orphaned)
	        bprintf(pip->efp,
	            "%s: exiting due to being orphaned\n",
	            pip->progname) ;

	} else {

	    if (pip->f.log) {

		if (rs == SR_IO) {
	            logfile_printf(&pip->lh,
			"exiting lost terminal (%d)",rs) ;

		} else
	            logfile_printf(&pip->lh,
			"exiting unknown (%d)",rs) ;

	    }

	} /* end if (orphaned or interrupt) */

/* undo these signal handlers */

#ifdef	COMMENT
	uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = SIG_DFL ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGTERM,&sigs,NULL) ;

	uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = SIG_DFL ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGHUP,&sigs,NULL) ;

	uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = SIG_DFL ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGINT,&sigs,NULL) ;
#endif /* COMMENT */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("daytimer: ret rs=%d\n",rs) ;
#endif

/* we are out of here */
done:
ret0:
	return rs ;
}
/* end subroutine (daytimer) */


/* local subroutines */


static void int_all(sn)
int	sn ;
{


	if_all = TRUE ;
	signal_num = sn ;
}
/* end subroutine (int_all) */



