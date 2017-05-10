/* watch */

/* watch the specified directory */
/* version %I% last modified %G% */


#define	CF_DEBUG	1
#define	CF_STALE	0


/* revision history:

	= 1991-09-10, David A­D­ Morano

	This subroutine was adopted from the DWD program.


*/

/* Copyright © 1991 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine is responsible for watching the specified
	directory.

	Arguments:

	- slp	service table list pointer
	- jlp	job table list pointer

	Returns:

	OK	doesn't really matter in the current implementation


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stropts.h>
#include	<poll.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<vecelem.h>
#include	<vecstr.h>
#include	<srvfile.h>
#include	<localmisc.h>

#include	"jobdb.h"
#include	"config.h"
#include	"defs.h"
#include	"configfile.h"


/* local defines */

#define	W_OPTIONS	(WUNTRACED | WNOHANG)


/* external subroutines */

extern int	cfdec() ;
extern int	getpwd(char *,int) ;
extern int	jobdb_add(), jobdb_search(), jobdb_findpid() ;
extern int	handlejob_start() ;
extern int	checklockfile() ;

extern char	*timestr_log(), *timevalstr_ulog(struct timeval *,char *) ;

extern void	handlejob_end() ;


/* externals variables */

extern struct global	g ;

extern int		errno ;


/* forward references */

int		intcmp() ;

void		makelogid() ;

static struct dirent	*lreaddir_r() ;

static int		dirtest() ;

static void		int_term() ;


/* local global variables */


/* local structures */


/* local variables */

/* built-in function table */
#define	FUN_REPORT	0
#define	FUN_EXIT	1

static char	*funtab[] = {
	"REPORT",
	"EXIT",
	NULL
} ;


/* exported subroutines */


int watch(slp,jlp)
struct vecelem	*slp ;
JOBDB		*jlp ;
{
	DIR		*dirp ;

	struct pollfd	fds[1] ;

	struct dirent	*dep ;

	struct ustat	isb, ssb, dsb, jsb ;

	struct timeval	tod ;

	struct sigaction	sigs ;

	struct jobentry		je, *jep ;

	struct srventry		*sep ;

	struct sortlist		timelist ;

	struct watchstate	ws, *wsp = &ws ;

	pid_t		pid ;

	sigset_t	signalmask ;

	time_t		daytime ;
	time_t		t_srvtab = 1, t_dir = 1 ;
	time_t		t_lockcheck = 1 ;
	time_t		t_pidcheck = 1 ;

	int	i, rs, len ;
	int	sleeptime ;
	int	polltimeout ;
	int	child_stat ;
	int	dfd, ifd, sfd ;
	int	f_directory ;
	int	f_interrupt = g.f.interrupt ;
	int	f_srvtab = g.f.srvtab ;
	int	w, s ;

	char	de[sizeof(struct dirent) + _POSIX_PATH_MAX + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1], timebuf2[TIMEBUFLEN + 1] ;
	char	*dnp ;


#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("watch: entered\n") ;
#endif

	wsp->logid_count = 0 ;
	for (i = 0 ; i < JOBSTATE_MAX ; i += 1) wsp->c[i] = 0 ;


	if (g.polltime > MAXSLEEPTIME)
	    g.polltime = MAXSLEEPTIME ;

	sleeptime = g.polltime ;

	sortlistinit(&timelist,10,intcmp) ;

/* let's try to cache the directory FD */

	f_directory = FALSE ;
	if ((dfd = dirtest(-1,g.directory,&dsb)) >= 0)
	    f_directory = TRUE ;

	else
	    logfile_printf(&g.lh,"queue directory not accessible\n") ;


/* what about an interrupt file ? */

	if (g.f.interrupt &&
	    ((ifd = open(g.interrupt,O_RDWR,0664)) >= 0)) {

	    fds[0].fd = ifd ;
	    fds[0].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;

	} else
	    g.f.interrupt = FALSE ;

	f_interrupt = g.f.interrupt ;


/* what about the service table */

	sfd = -1 ;
	if (g.f.srvtab) {

	    if (access(g.srvtab,R_OK) >= 0)
	        sfd = open(g.srvtab,O_RDONLY,0666) ;

	    t_srvtab = 1 ;
	    if ((sfd >= 0) && (fstat(sfd,&ssb) >= 0))
	        t_srvtab = ssb.st_mtime ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("watch: starting sfd=%d\n",sfd) ;
#endif

	} /* end if (we have a SRVTAB file) */


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_term ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGTERM,&sigs,NULL) ;



/* let's go ! */

	g.f_exit = FALSE ;

top:


/* has the service table file changed since we last looked ? */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("watch: before srvtab, c.watch=%d\n",
	        wsp->c[JOBSTATE_WATCH]) ;
#endif

	if (f_srvtab) {

#if	CF_DEBUG
	    if (g.debuglevel > 0) {

	        gettimeofday(&tod,NULL) ;

	        debugprintf("watch: %s inside srvtab if, sfd=%d\n",
	            timevalstr_ulog(&tod,timebuf),sfd) ;

	    }
#endif /* CF_DEBUG */

	    if (sfd < 0)
	        sfd = u_open(g.srvtab,O_RDONLY,0666) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("watch: after possible SRVTAB open, sfd=%d\n",
	            sfd) ;
#endif

	    if (sfd >= 0) {

	        if ((fstat(sfd,&ssb) >= 0) && (ssb.st_mtime > t_srvtab)) {

	            if (g.f.srvtab)
	                logfile_printf(&g.lh,
	                    "%s the service file changed\n",
	                    timestr_log(ssb.st_mtime,timebuf)) ;

	            else
	                logfile_printf(&g.lh,
	                    "%s the service file has returned\n",
	                    timestr_log(ssb.st_mtime,timebuf)) ;

	            (void) time(&daytime) ;

	            t_srvtab = ssb.st_mtime ;
	            while ((fstat(sfd,&ssb) >= 0) &&
	                ((daytime - ssb.st_mtime) < SRVIDLETIME)) {

	                sleep(1) ;

	                (void) time(&daytime) ;

	            } /* end while */

	            g.f.srvtab = FALSE ;
	            if (access(g.srvtab,R_OK) >= 0) {

	                srvfree(slp) ;

	                if ((rs = srvinit(slp,g.srvtab)) >= 0) {

	                    g.f.srvtab = TRUE ;
	                    if (t_dir > 1) t_dir -= 1 ;

	                }

	                (void) time(&daytime) ;

	                logfile_printf(&g.lh,
	                    "%s the new service file is %s\n",
	                    timestr_log(daytime,timebuf),
	                    (g.f.srvtab) ? "OK" : "BAD") ;

	            } else {

	                logfile_printf(&g.lh,
	                    "%s the new service file is not accessible\n",
	                    timestr_log(ssb.st_mtime,timebuf)) ;

	                close(sfd) ;

	                sfd = -1 ;

	            } /* end if (accessibility of the new one) */

	        } /* end if (service file changed) */

	    } else {

	        if (g.f.srvtab) {

	            g.f.srvtab = FALSE ;
	            logfile_printf(&g.lh,
	                "%s service file went away\n",
	                timestr_log(ssb.st_mtime,timebuf)) ;

	            close(sfd) ;

	            sfd = -1 ;

	        }

	    } /* end if (service file present or not) */

	} /* end if (we're supposed to have a service table file) */


/* is our directory even there ? */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("watch: before checking directory\n") ;
#endif

	(void) time(&daytime) ;

	dfd = dirtest(dfd,g.directory,&dsb) ;

	if ((dfd >= 0) &&
	    (((daytime - dsb.st_mtime) > (sleeptime * 5)) ||
	    (dsb.st_mtime > t_dir))) {

	    close(dfd) ;

	    dfd = dirtest(-1,g.directory,&dsb) ;

	} /* end if */

	if (dfd >= 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("watch: directory seems to be there\n") ;
#endif

	    if (! f_directory) {

	        (void) time(&daytime) ;

	        f_directory = TRUE ;
	        logfile_printf(&g.lh,
	            "%s directory came back\n",
	            timestr_log(daytime,timebuf)) ;

	        sleeptime = g.polltime ;

	    }

	} else {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("watch: directory gone ?\n") ;
#endif

	    if (f_directory) {

	        (void) time(&daytime) ;

	        f_directory = FALSE ;
	        logfile_printf(&g.lh,
	            "%s directory went away\n",
	            timestr_log(daytime,timebuf)) ;

	    }

	    sleeptime = (sleeptime * 2) ;
	    if (sleeptime > (10 * g.polltime))
	        sleeptime = (10 * g.polltime) ;

	    if (sleeptime > MAXSLEEPTIME)
	        sleeptime = MAXSLEEPTIME ;

	} /* end if (directory test) */


/* has the directory modification time changed ? */

	if ((dfd >= 0) && 
	    ((wsp->c[JOBSTATE_WATCH] > 0) || 
	    (wsp->c[JOBSTATE_STALE] > 0) || 
	    (dsb.st_mtime > t_dir))) {

	    close(dfd) ;

	    dfd = -1 ;

#if	CF_DEBUG
	    if (g.debuglevel > 0) {

	        gettimeofday(&tod,NULL) ;

	        if (dsb.st_mtime > t_dir) {

	            debugprintf("watch: %s dir changed\n",
	                timevalstr_ulog(&tod,timebuf)) ;

	            debugprintf("watch: dirstat=%s dirtime=%s\n",
	                timestr_log(dsb.st_mtime,timebuf),
	                timestr_log(t_dir,timebuf2)) ;

	        } else if (wsp->c[JOBSTATE_WATCH] > 0)
	            debugprintf("watch: DIR c.watch=%d\n",
	                wsp->c[JOBSTATE_WATCH]) ;

	        else
	            debugprintf("watch: DIR don't know\n") ;

	    }
#endif /* CF_DEBUG */

	    t_dir = dsb.st_mtime ;
	    if ((dirp = opendir(g.directory)) != NULL) {

	        while ((dep = (struct dirent *) lreaddir_r(dirp,
	            (struct dirent *) de)) != NULL) {

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                debugprintf("watch: top of inner while, entry=\"%s\"\n",
	                    dep->d_name) ;
#endif

	            dnp = dep->d_name ;

/* do not play with the "special" entries ! (this is a fast way of deciding) */

	            if (dnp[0] == '.') {

	                if (dnp[1] == '\0') continue ;

	                if ((dnp[1] == '.') && (dnp[2] == '\0'))
	                    continue ;

	            }

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                debugprintf("watch: about to search for entry=\"%s\"\n",
	                    dep->d_name) ;
#endif

	            sprintf(tmpfname,"%s/%s",g.directory,dnp) ;

	            if (stat(tmpfname,&jsb) < 0) {

	                unlink(tmpfname) ;

	                continue ;
	            }

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                debugprintf("watch: good stat on file=%s\n",
	                    dnp) ;
#endif

/* do we already have this entry in our table list */

	            if ((i = jobdb_search(jlp,dnp,&jep)) < 0) {

/* we have not seen this job before ! */

#if	CF_DEBUG
	                if (g.debuglevel > 0)
	                    debugprintf("watch: new job, filename=%s\n",
	                        dnp) ;
#endif

	                strcpy(je.filename,dnp) ;

	                makelogid(wsp,&je.logid) ;

#if	CF_DEBUG
	                if (g.debuglevel > 0)
	                    debugprintf("watch: logid=%s\n",je.logid) ;
#endif

	                je.size = jsb.st_size ;
	                je.atime = jsb.st_mtime ;
	                je.mtime = jsb.st_mtime ;

	                logfile_setid(&g.lh,je.logid) ;

#if	CF_DEBUG
	                if (g.debuglevel > 0)
	                    debugprintf("watch: set logid\n") ;
#endif

	                (void) time(&daytime) ;

	                logfile_printf(&g.lh, "%s job entering\n",
	                    timestr_log(daytime,timebuf)) ;

#if	CF_DEBUG
	                if (g.debuglevel > 0)
	                    debugprintf("watch: made log entry\n") ;
#endif

	                logfile_printf(&g.lh,
	                    "f=%s\n",je.filename) ;

	                je.state = JOBSTATE_WATCH ;
	                wsp->c[JOBSTATE_WATCH] += 1 ;

	                i = jobdb_add(jlp,&je,sizeof(struct jobentry)) ;


#if	CF_DEBUG
	                if (g.debuglevel > 0)
	                    debugprintf("watch: added job to job table\n") ;
#endif

	                logfile_setid(&g.lh,g.logid) ;

	            } else {

/* handle existing jobs */

#if	CF_DEBUG
	                if (g.debuglevel > 0)
	                    debugprintf("watch: found job \"%s\"\n",dnp) ;
#endif

	                if (jep->state == JOBSTATE_WATCH) {

	                    (void) time(&daytime) ;

	                    if ((jsb.st_size > jep->size) ||
	                        (jsb.st_mtime > jep->mtime)) {

#if	CF_DEBUG
	                        if (g.debuglevel > 0)
	                            debugprintf("watch: file changed\n") ;
#endif

	                        jep->size = jsb.st_size ;
	                        jep->mtime = jsb.st_mtime ;

	                    } else if (daytime > 
	                        (jep->mtime + JOBIDLETIME)) {

#if	CF_DEBUG
	                        if (g.debuglevel > 0) debugprintf(
	                            "watch: file \"%s\" not changed\n",
	                            jep->filename) ;
#endif

	                        logfile_setid(&g.lh,jep->logid) ;

	                        logfile_printf(&g.lh,"%s processing job\n",
	                            timestr_log(daytime,timebuf)) ;

#if	CF_DEBUG
	                        if (g.debuglevel > 0)
	                            debugprintf("watch: about to start job\n") ;
#endif

	                        if ((rs = handle_new(wsp,jlp,jep,&jsb,slp))
	                            < 0) {

#if	CF_DEBUG
	                            if (g.debuglevel > 0)
	                                debugprintf(
	                                    "watch: deleting job, rs=%d\n",
	                                    rs) ;
#endif

	                            handle_del(jlp,-1,jep) ;

	                        }

	                        logfile_setid(&g.lh,g.logid) ;

	                    } /* end if (file changing or idle) */

	                } else if ((jep->state == JOBSTATE_STALE) &&
	                    (t_srvtab > jep->mtime)) {

/* can we get rid of any old stale jobs */

	                    if ((rs = srvsearch(slp,jep->service,
	                        &sep)) >= 0) {

	                        logfile_setid(&g.lh,jep->logid) ;

	                        logfile_printf(&g.lh, "%s processing job\n",
	                            timestr_log(daytime,timebuf)) ;

#if	CF_DEBUG
	                        if (g.debuglevel > 0)
	                            debugprintf("watch: about to start job\n") ;
#endif

	                        if (handlejob_start(jep,NULL,&jsb,sep)
	                            >= 0) {

#if	CF_DEBUG
	                            if (g.debuglevel > 0)
	                                debugprintf("watch: started job\n") ;
#endif

	                            jep->state = JOBSTATE_STARTED ;
	                            wsp->c[JOBSTATE_STALE] -= 1 ;
	                            wsp->c[JOBSTATE_STARTED] += 1 ;

	                        } /* end if */

	                    } /* end if (we have a service for this job) */

	                } /* end if (handling existing jobs) */

	            } /* end if (new or old job) */

	        } /* end while (looping through directory entries) */

	        closedir(dirp) ;

	    } /* end if (opened directory) */

	} /* end if (scanning directory) */


/* scan for any completed jobs */

	if (wsp->c[JOBSTATE_STARTED] > 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("watch: scanning for completed jobs\n") ;
#endif

	    while ((pid = waitpid(-1,&child_stat,W_OPTIONS)) > 0) {

	        if ((i = jobdb_findpid(jlp,pid,&jep)) >= 0) {

	            handlejob_getstat(jep,&w,&s) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("watch: job finished, w=%d s=%d\n",
			w,s) ;
#endif

	            if ((w == 2) && (s >= 0)) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("watch: ending this job\n") ;
#endif

	                (void) time(&daytime) ;

	                logfile_setid(&g.lh,jep->logid) ;

	                logfile_printf(&g.lh, "%s job completed, es=%d\n",
	                    timestr_log(daytime,timebuf),
	                    child_stat & 255) ;

	                handlejob_end(jep,child_stat) ;

	                handle_del(jlp,i,NULL) ;

	                wsp->c[JOBSTATE_STARTED] -= 1 ;

	                logfile_setid(&g.lh,g.logid) ;

	            } else {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("watch: making this job stale\n") ;
#endif

	                jep->state = JOBSTATE_STALE ;
	                wsp->c[JOBSTATE_STARTED] -= 1 ;
	                wsp->c[JOBSTATE_STALE] += 1 ;

	            }

	        } /* end if */

	    } /* end while (processing job completions) */

	} /* end if (scanning for completed jobs) */


/* scan to see if the stale jobs can be started */

#if	CF_STALE
	if ((wsp->c[JOBSTATE_STALE] > 0) && g.f.srvtab && f_srvtab) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("watch: checking the stales\n") ;
#endif

	    for (i = 0 ; (rs = jobdb_get(jlp,i,&jep)) >= 0 ; i += 1) {

	        if (jep == NULL) continue ;

	        if (jep->state != JOBSTATE_STALE) continue ;

#if	CF_DEBUG
	        if (g.debuglevel > 0)
	            debugprintf("watch: stale=%s\n",jep->filename) ;
#endif

	        sprintf(tmpfname,"%s/%s",g.directory,jep->filename) ;

	        if (access(tmpfname,R_OK) < 0) continue ;

	        if (srvsearch(slp,jep->service,&sep) >= 0) {

	            logfile_setid(&g.lh,jep->logid) ;

	            logfile_printf(&g.lh, "%s processing old job\n",
	                timestr_log(daytime,timebuf)) ;

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                debugprintf("watch: about to start job\n") ;
#endif

	            if (handlejob_start(jep,NULL,&jsb,sep) >= 0) {

#if	CF_DEBUG
	                if (g.debuglevel > 0)
	                    debugprintf("watch: started job\n") ;
#endif

	                jep->state = JOBSTATE_STARTED ;
	                wsp->c[JOBSTATE_STALE] -= 1 ;
	                wsp->c[JOBSTATE_STARTED] += 1 ;

	            } /* end if */

	            logfile_setid(&g.lh,g.logid) ;

	        } /* end if */

	    } /* end for */

	} /* end if */
#endif /* CF_STALE */


/* maintenance the lock file */

	if ((g.lockfp != NULL) && 
	    ((daytime - t_lockcheck) >= (MAXSLEEPTIME - 1))) {

	    if ((rs = checklockfile(g.lockfp,g.lockfile,daytime)) != 0) {

	        logfile_printf(&g.lh,
	            "%s other program has my lock file, other PID=%d\n",
	            timestr_log(daytime,timebuf),
	            rs) ;

	        goto badlockfile ;
	    }

	    t_lockcheck = daytime ;

	}


/* maintenance the PID mutex lock file */

	if ((g.pidfp != NULL) && 
	    ((daytime - t_pidcheck) >= (MAXSLEEPTIME - 1))) {

	    if ((rs = checklockfile(g.pidfp,g.pidfile,daytime)) != 0) {

	        logfile_printf(&g.lh,
	            "%s other program has my PID file, other PID=%d\n",
	            timestr_log(daytime,timebuf),
	            rs) ;

	        goto badpidfile ;
	    }

	    t_pidcheck = daytime ;

	}




/* overhead scanning functions */

	if ((wsp->c[JOBSTATE_WATCH] == 0) && 
	    (wsp->c[JOBSTATE_STARTED] == 0) &&
	    (sleeptime < g.polltime)) sleeptime += 5 ;


/* wait for a specified amount of time */

	if (f_interrupt) {

#if	CF_DEBUG
	    if (g.debuglevel > 0) {

	        gettimeofday(&tod,NULL) ;

	        debugprintf("watch: %s about to wait, sleeptime=%d\n",
	            timevalstr_ulog(&tod,timebuf),sleeptime) ;
	    }
#endif

	    polltimeout = sleeptime * 1000 ;

	    if ((rs = poll(fds,1,polltimeout)) < 0)
	        rs = (- errno) ;

	    if (rs > 0) {

	        char	eventbuf[80] ;

#if	CF_DEBUG
	        if (g.debuglevel > 0) {

	            gettimeofday(&tod,NULL) ;

	            debugprintf("watch: %s got an interrupt\n",
	                timevalstr_ulog(&tod,timebuf)) ;

	            debugprintf("watch: revents %s\n",
	                reventstr(fds[0].revents,eventbuf)) ;

	        }
#endif /* CF_DEBUG */

	        if ((rs = read(ifd,tmpfname,MAXPATHLEN)) < 0)
	            rs = (- errno) ;

#if	CF_DEBUG
	        if (g.debuglevel > 0) {

	            gettimeofday(&tod,NULL) ;

	            debugprintf("watch: %s back from read, rs=%d\n",
	                timevalstr_ulog(&tod,timebuf),rs) ;

	        }

	    } else {

	        if (g.debuglevel > 0) {

	            gettimeofday(&tod,NULL) ;

	            debugprintf("watch: %s back from poll, rs=%d\n",
	                timevalstr_ulog(&tod,timebuf),rs) ;

	        }

	        sleep(4) ;
#endif /* CF_DEBUG */

	    } /* end if (poll) */

	} else
	    sleep(sleeptime) ;

#if	CF_DEBUG
	if (g.debuglevel > 0) {

	    gettimeofday(&tod,NULL) ;

	    debugprintf("watch: %s we are starting a scan cycle\n",
	        timevalstr_ulog(&tod,timebuf)) ;
	}
#endif


	if (! g.f_exit) goto top ;


	time(&daytime) ;

	logfile_printf(&g.lh,"%s program exiting\n",
	    timestr_log(daytime,timebuf)) ;


	if (g.f.interrupt) close(ifd) ;

	if (dfd >= 0) close(dfd) ;

	if (sfd >= 0) close(sfd) ;

	return OK ;

killed:
badlockfile:
badpidfile:
	if (g.f.interrupt) close(ifd) ;

	if (dfd >= 0) close(dfd) ;

	if (sfd >= 0) close(sfd) ;

	return BAD ;
}
/* end subroutine (watch) */


/* "local" readdir_r */
static struct dirent *lreaddir_r(dirp,de)
DIR		*dirp ;
char		de[] ;
{


#ifdef	SYSV
	return (struct dirent *) readdir_r(dirp,
	    (struct dirent *) de) ;
#else
	return (struct dirent *) readdir(dirp) ;
#endif

}
/* end subroutine (lreaddir_r) */


void makelogid(wsp,logid)
struct watchstate	*wsp ;
char			logid[15] ;
{
	char	buf[256] ;


	sprintf(buf,"%d.%d",g.pid,(wsp->logid_count)++) ;

	strwcpy(logid,buf,14) ;

}


#ifdef	COMMENT

static void internal(filename)
char	filename[] ;
{
	int	i, srs = FALSE ;

	char	*cp ;


	if (((cp = strchr(filename,'.')) != NULL) &&
	    isupper(filename) &&
	    (strcmp(cp + 1,INTERNALSUFFIX) == 0) &&
	    ((i = matstr(funtab,filename,cp - filename)) >= 0)) {

	    switch (i) {

	    case FUN_REPORT:

	        break ;

	    case FUN_EXIT:

	        break ;

	    } /* end switch */

	    srs = TRUE ;
	}

	return srs ;
}

#endif /* COMMENT */


static int dirtest(dfd,n,sbp)
int		dfd ;
char		n[] ;
struct ustat	*sbp ;
{
	int	rs ;


	if (dfd < 0)
	    dfd = open(n,O_RDONLY,0666) ;

	if (dfd < 0) return (- errno) ;

	if ((rs = fstat(dfd,sbp)) < 0) rs = (- errno) ;

	if (rs >= 0) return dfd ;

	close(dfd) ;

	return rs ;
}
/* end subroutine (dirtest) */


static void int_term(sn)
int	sn ;
{


	g.f_exit = TRUE ;
}


int intcmp(a,b)
int	a, b ;
{

	return (a - b) ;
}



