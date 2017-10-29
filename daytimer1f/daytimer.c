/* daytimer */

/* day timer subroutine */


#define	F_DEBUG		1		/* switchable debug print-outs */
#define	F_POLL		1
#define	F_ORPHANED	1		/* check for becomming an orphan */


/* revision history :

	= 88/02/01, David A­D­ Morano

	This subroutine was originally written.


	= 98/05/01, David A­D­ Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


	= 98/12/01, David A­D­ Morano

	This subroutine has been updated to use the new Lock File
	Manager (lfm) and also the new display manager object for
	management of the display.


*/



/************************************************************************

	This is the subroutine which performs the active work for
	the 'daytime' program.



*************************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
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

#include	"misc.h"
#include	"mailfiles.h"
#include	"config.h"
#include	"defs.h"
#include	"display.h"



/* local defines */



/* external subroutines */

extern int	isproc(pid_t) ;
extern int	checklockfile(struct proginfo *,bfile *,char *,char *,
				time_t,pid_t) ;

extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* forward references */

static void	int_all(int) ;


/* local file-scope variables */

static int	f_exit = FALSE ;
static int	signal_num = 0 ;







int daytimer(pip,up,lfp,t_offset,t_timeout,t_refresh,f_statdisplay,mfp)
struct proginfo	*pip ;
struct userinfo	*up ;
LFM		*lfp ;
time_t		t_offset, t_timeout, t_refresh ;
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
	time_t	t_access ;
	time_t	t_mail = 0L ;
	time_t	t_checklocks = 0 ;

	int	rs = SR_OK, i ;
	int	tic = 0, mail_tics = MAIL_TICS ;
	int	state = S_NOMAIL ;
	int	childstat ;
	int	nfds = 0 ;
	int	fd_display = FD_STDOUT ;
	int	f_orphaned = FALSE ;
	int	f_newmail = FALSE ;
	int	f_dtime = FALSE ;
	int	f_mtime = FALSE ;
	int	f_displaying = TRUE ;
	int	f_blank = FALSE ;
	int	f_inputavail = FALSE ;
	int	f_update = FALSE ;

	char	mailbuf[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	timebuf[100] ;
#if	F_DEBUG
	char	timebuf1[100] ;
	char	timebuf2[100] ;
#endif
	char	*maildir ;
	char	*mailfname = NULL ;


#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("daytimer: entered, refresh=%ld\n",
	        t_refresh) ;
#endif

/* initialization */

	if ((rs = mailfiles_count(mfp)) <= 0) {

	    if ((mailfname = getenv("MAIL")) == NULL) {

	        if ((maildir = getenv("MAILDIR")) == NULL)
	            maildir = MAILDIR ;

	        strcat(mailbuf,maildir) ;

	        strcat(mailbuf,"/") ;

	        strcat(mailbuf,up->username) ;

	        mailfname = mailbuf ;

	    } /* end if */

#if	F_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("daytimer: default mailfile=%\n",mailfname) ;
#endif

	} /* end if (trying to get a MAIL file to monitor) */

#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("daytimer: mailfiles count=%d\n",rs) ;
#endif


/* set the signals that we want to catch */

	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGTERM,&sigs,NULL) ;


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGHUP,&sigs,NULL) ;


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGINT,&sigs,NULL) ;



/* set the default timeout time */

	if (t_timeout < 10)
	    t_timeout = 10 ;


	u_time(&daytime) ;


/* get the access time of our output file */

	if ((rs = u_fstat(fd_display,&sb)) > 0)
	    goto badstat ;

	t_access = sb.st_atime ;


/* initialize the POLL structure */

#if	F_POLL
	fds[0].fd = fd_display ;
	fds[0].events = POLLIN ;
	fds[1].fd = -1 ;
	fds[1].events = POLLIN ;
	nfds = 1 ;
#endif


/* initialize the display object */

	rs = display_init(&d,fd_display,
	    f_statdisplay,t_refresh,t_offset) ;

	if (rs < 0)
	    goto baddisplay ;


/* OK, go into infinite loop mode */

	i = 0 ;
	f_exit = FALSE ;
	while ((! f_orphaned) && (! f_exit)) {

		int	f_daytime ;


#if	F_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("daytimer: tic=%d\n",tic) ;
#endif

/* get mail status */

	    if ((tic % mail_tics) == 0) {

	        int	f_changed = FALSE ;


	        if (mailfname == NULL) {

#if	F_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("daytimer: checking 'mailfiles' list\n") ;
#endif

	            if (mailfiles_check(mfp) > 0) {

	                f_changed = TRUE ;
			f_daytime = TRUE ;
	                (void) u_time(&t_mail) ;

	            }

	        } else {

#if	F_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("daytimer: checking the default mailfname\n") ;
#endif

	            rs = u_stat(mailfname,sp) ;

	            t_mail = sp->st_mtime ;
	            if ((rs >= 0) && (sp->st_size > 0))
	                f_changed = TRUE ;

	        }

	        if (f_changed) {

	            state = S_MAIL ;
	            mail_tics = 1 ;		/* fast update */
	            if (! f_newmail) {

	                f_newmail = TRUE ;

#if	F_DEBUG
	                if (pip->debuglevel > 1)
	                    eprintf("daytimer: mail=%s clock=s\n",
	                        timestr_log(t_mail,timebuf1),
	                        timestr_log(daytime,timebuf2)) ;
#endif

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


/* get time */

		if (! f_daytime)
	    (void) u_time(&daytime) ;


/* get terminal access time */

#if	F_DEBUG
	    if (pip->debuglevel > 1) {
	        eprintf("daytimer: terminal access time calculations\n") ;
	        eprintf("daytimer: f_update=%d\n",f_update) ;
	}
#endif

	    if (f_update) {

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("daytimer: f_update\n") ;
#endif

	        f_update = FALSE ;
	        t_access = daytime ;

	    } else if (daytime > (t_access + t_timeout)) {

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("daytimer: past access timeout\n") ;
#endif

	        if ((rs = u_fstat(fd_display,&sb)) > 0)
	            goto badstat ;

	        t_access = sb.st_atime ;

	    } /* end if */

	    f_dtime = FALSE ;
	    if (daytime <= (t_access + t_timeout)) {

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("daytimer: f_dtime, access=%s clock=%s\n",
	                timestr_log(t_access,timebuf1),
	                timestr_log(daytime,timebuf2)) ;
#endif

	        f_dtime = TRUE ;

	    }

	    f_mtime = FALSE ;
	    if (daytime <= (t_mail + DEF_MAILTIME))
	        f_mtime = TRUE ;


	    f_blank = FALSE ;
	    if (f_displaying && ((! f_mtime) && (! f_dtime))) {

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("daytimer: f_displaying and not other stuff\n") ;
#endif

	        f_displaying = FALSE ;
	        f_blank = TRUE ;

	    } else
	        f_displaying = f_mtime || f_dtime ;

	    if (state == S_MAILDONE) 
		state = S_NOMAIL ;


/* formulate the status display and write */

#if	F_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("daytimer: f_dtime=%d t_mtime=%d\n",
	            f_dtime,f_mtime) ;
#endif

	    if (f_displaying) {

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("daytimer: display_show()\n") ;
#endif

	        rs = display_show(&d,daytime,f_newmail) ;

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("daytimer: display() rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            break ;

	    } /* end if (updating display) */


	    if (f_blank) {

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("daytimer: display_blank()\n") ;
#endif

	        rs = display_blank(&d) ;

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("daytimer: blanked rs=%d\n",rs) ;
#endif

	    } /* end if (blank requested) */


/* lock file checks */

#if	F_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("daytimer: lock file checks\n") ;
#endif

	    if (pip->f.lockfile || pip->f.pidfile) {

		if (! f_daytime)
	        (void) u_time(&daytime) ;

	        if (daytime > (t_checklocks + (LOCKTIMEOUT / 2))) {

	            t_checklocks = daytime ;

#if	F_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("daytimer: checking locks daytime=%d (%s)\n",
	                    daytime,timestr_log(daytime,timebuf)) ;
#endif /* F_DEBUG */

	            if (pip->f.lockfile) {

	                rs = lfm_check(lfp,NULL,daytime) ;

#if	F_DEBUG
	                if (pip->debuglevel > 1)
	                    eprintf("daytimer: lfm rs=%d\n",
	                        rs) ;
#endif /* F_DEBUG */

	                if (rs != 0) 
				goto lockconflict ;

	            } /* end if (lockfile) */

#if	F_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("daytimer: past lock check\n") ;
#endif /* F_DEBUG */

	            if (pip->f.pidfile) {

	                rs = checklockfile(pip,pip->pfp,pip->pidfname, 
	                    pip->banner,daytime,pip->pid) ;

	                if (rs != 0)
				goto pidconflict ;

	            } /* end if (pidfile) */

#if	F_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("daytimer: past PID lock check\n") ;
#endif /* F_DEBUG */

	        } /* end if (we have a time check interval */

	    } /* end if (checking lock files) */


#if	F_DEBUG
	            if (pip->debuglevel > 1)
	                display_check(&d,daytime) ;
#endif


#if	F_POLL
	    if (! f_displaying) {

	        rs = u_poll(fds,1,1000 * TIME_SLEEP) ;

#if	F_DEBUG
	        if (pip->debuglevel > 1) {

	            eprintf("daytimer: poll debug\n") ;

	            if (rs > 0) {

	                if (fds[0].revents & POLLIN)
	                    eprintf("daytimer: returned due to POLL\n") ;

	                else
	                    eprintf("daytimer: returned due to %08X\n",
	                        fds[0].revents) ;

	            } else
	                eprintf("daytimer: returned normally\n") ;

	        }
#endif /* F_DEBUG */

	        if (rs >= 0) {

	            if (fds[0].revents & POLLIN) {

#if	F_DEBUG
	                if (pip->debuglevel > 1)
	                    eprintf("daytimer: POLLIN f_avail=%d\n",
	                        f_inputavail) ;
#endif

	                if (! f_inputavail) {

	                    f_inputavail = TRUE ;
	                    f_update = TRUE ;

#if	F_DEBUG
	                if (pip->debuglevel > 1)
	                    eprintf("daytimer: setting f_update=TRUE\n") ;
#endif

	                } else
	                    sleep(TIME_SLEEP) ;

	            } else {

#if	F_DEBUG
	                if (pip->debuglevel > 1)
	                    eprintf("daytimer: input not available\n") ;
#endif

	                f_inputavail = FALSE ;

	            }
	        }

	    } else
	        sleep(TIME_SLEEP) ;
#else
	    sleep(TIME_SLEEP) ;
#endif /* F_POLL */

		f_daytime = FALSE ;


/* see if we have been orphaned */

	if (((tic % 3) == 0) && (! f_orphaned)) {

	    ppid = getppid() ;

	    pgid = getpgrp() ;

#if	F_ORPHANED
	    if ((ppid == 1) && (up->pid == pgid))
	        f_orphaned = TRUE ;
#endif

	} /* end if (orphan check) */

	if (((tic % 10) == 0) && (! f_orphaned)) {

		if (! isproc(pip->sid))
			f_orphaned = TRUE ;

	} /* end if (orphan check) */


#ifdef	COMMENT
	    if (i < 10)
	        (void) u_wait(&childstat) ;
#endif

/* bottom of loop processing */

	    tic = (tic + 1) % 1000 ;
	    i += 1 ;

	} /* end while (main loop) */


#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("daytimer: after while loop\n") ;
#endif

	display_free(&d) ;

#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("daytimer: after display free\n") ;
#endif


/* log termination status */

	if (f_orphaned || f_exit) {

#if	F_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("daytimer: orphaned or exit\n") ;
#endif

	    (void) u_time(&daytime) ;

	    i = bufprintf(buf,BUFLEN,"%s",
			timestr_log(daytime,timebuf)) ;

	    if (f_orphaned)
	        i += sprintf(buf + i," orphaned") ;

	    if (f_exit)
	        i += bufprintf(buf + i,(BUFLEN - i)," exited (TERM)\n") ;

#ifdef	COMMENT
	    i += sprintf(buf + i," rs=%d\n",rs) ;
#endif

	    logfile_printf(&pip->lh,buf) ;

	} /* end if */

/* we are done */

	if (pip->debuglevel > 0) {

	    if (rs < 0)
	        bprintf(pip->efp,
	            "%s: exiting due to write failure (rs %d)\n",
	            pip->progname,rs) ;

	    if (f_orphaned)
	        bprintf(pip->efp,
	            "%s: exiting due to being orphaned\n",
	            pip->progname) ;

	} /* end if */


#ifdef	COMMENT

/* undo these signal handlers */

	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = SIG_DFL ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGTERM,&sigs,NULL) ;


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = SIG_DFL ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGHUP,&sigs,NULL) ;


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = SIG_DFL ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGINT,&sigs,NULL) ;

#endif /* COMMENT */



#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("daytimer: about to done rs=%d\n",rs) ;
#endif

/* we are out of here */
done:
	return rs ;

/* bad returns come here */
badstat:
	bprintf(pip->efp,"%s: could not get status of terminal (rs %d)\n",
	    pip->progname,rs) ;

badret:
	return rs ;

lockconflict:
	bprintf(pip->efp,"%s: detected a lock conflict (rs %d)\n",
	    pip->progname,rs) ;

	goto done ;

pidconflict:
	bprintf(pip->efp,"%s: detected a PID lock conflict (rs %d)\n",
	    pip->progname,rs) ;

	goto done ;

baddisplay:
	bprintf(pip->efp,"%s: bad display FD (rs %d)\n",
	    pip->progname,rs) ;

	goto done ;

}
/* end subroutine (daytimer) */



/* LOCAL SUBROUTINES */



static void int_all(sn)
int	sn ;
{

	f_exit = TRUE ;
	signal_num = sn ;
}



