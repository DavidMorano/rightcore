/* watch */

/* watch the specified directory */
/* version %I% last modified %G% */


#define	F_DEBUG		1
#define	F_STALE		0


/* revision history :

	= Dave Morano, September 1991
	This program was originally written.


*/



/*****************************************************************************

	This subroutine is responsible for watching the specified
	directory.

	This subroutine is called with the folowing arguments :

	- slp	service table list pointer
	- jlp	job table list pointer


	Returns :
	OK	doesn't really matter in the current implementation



*****************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/time.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<ftw.h>
#include	<errno.h>
#include	<limits.h>
#include	<string.h>
#include	<stropts.h>
#include	<poll.h>
#include	<dirent.h>

#include	<bio.h>
#include	<logfile.h>
#include	<vecelem.h>
#include	<vecstr.h>
#include	<bitops.h>
#include	<system.h>
#include	<directory.h>

#include	"misc.h"
#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"srvfile.h"



/* local defines */

#define	W_OPTIONS	(WUNTRACED | WNOHANG)



/* external subroutines */

extern int	cfdec() ;
extern int	substring() ;
extern int	getpwd() ;
extern int	job_search(), job_start(), job_findpid(), job_end() ;
extern int	checklockfile() ;

extern char	*strbasename() ;
extern char	*timestr_log(), *timestr_ulog() ;
extern char	*strwcpy() ;


/* external variables */

extern struct global	g ;


/* local global variables */


/* local structures */

union dirslot {
	struct dirent	entry ;
	char		pad1[sizeof(struct dirent) + _POSIX_PATH_MAX + 1] ;
	char		pad2[sizeof(struct dirent) + MAXPATHLEN + 1] ;
} ;


/* forwards */

static int		dirtest() ;

static void		makejobid() ;
static void		int_term() ;


/* local data */

/* built-in function table */

#define	FUN_REPORT	0
#define	FUN_EXIT	1


static char	*funtab[] = {
	"REPORT",
	"EXIT",
	NULL
} ;




int watch(slp,jlp,maxjobs)
struct vecelem	*slp ;
struct vecelem	*jlp ;
int		maxjobs ;
{
	DIR		*dirp ;

	struct pollfd	fds[1] ;

	struct dirent	*dep ;

	struct stat	ssb, dsb, jsb ;

	struct timeval	tod ;

	struct sigaction	sigs ;

	struct jobentry		je, *jep ;

	struct srventry		*sep ;

	union dirslot	de ;

	pid_t		pid ;

	sigset_t	signalmask ;

	time_t		daytime ;
	time_t		t_srvtab = 1, t_dir = 1 ;
	time_t		t_lockcheck = 1 ;
	time_t		t_pidcheck = 1 ;

	int	i, rs ;
	int	sleeptime ;
	int	polltimeout ;
	int	nwatch = 0 ;
	int	nstarted = 0 ;
	int	nwaiting = 0 ;
	int	nstale = 0 ;
	int	child_stat ;
	int	jobid = 0 ;
	int	dfd, ifd, sfd ;
	int	f_directory ;
	int	f_interrupt = g.f.interrupt ;
	int	f_srvtab = g.f.srvtab ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[50], timebuf2[50] ;
	char	*dnp ;


#if	F_DEBUG
	if (g.debuglevel > 1)
	    eprintf("watch: entered\n") ;
#endif

	if (g.polltime > MAXSLEEPTIME)
	    g.polltime = MAXSLEEPTIME ;

	sleeptime = g.polltime ;

/* let's try to cache the directory FD */

	f_directory = FALSE ;
	if ((dfd = dirtest(-1,g.directory,&dsb)) >= 0)
	    f_directory = TRUE ;

	else
	    logprintf(&g.lh,"queue directory not accessible\n") ;


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

	    if (u_access(g.srvtab,R_OK) >= 0)
	        sfd = open(g.srvtab,O_RDONLY,0666) ;

	    t_srvtab = 1 ;
	    if ((sfd >= 0) && (fstat(sfd,&ssb) >= 0))
	        t_srvtab = ssb.st_mtime ;

#if	F_DEBUG
	    if (g.debuglevel > 1)
	        eprintf("watch: starting sfd=%d\n",sfd) ;
#endif

	} /* end if (we have a SRVTAB file) */


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_term ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGTERM,&sigs,NULL) ;



/* let's go ! */

	g.f_exit = FALSE ;
	while (! g.f_exit) {


/* has the service table file changed since we last looked ? */

#if	F_DEBUG
	    if (g.debuglevel > 1)
	        eprintf("watch: before srvtab, nwatch=%d\n",nwatch) ;
#endif

	    if (f_srvtab) {

#if	F_DEBUG
	        if (g.debuglevel > 1) {

	            gettimeofday(&tod,NULL) ;

	            eprintf("watch: %s inside srvtab if, sfd=%d\n",
	                timestr_ulog(&tod,timebuf),sfd) ;

	        }
#endif /* F_DEBUG */

	        if (sfd < 0)
	            sfd = open(g.srvtab,O_RDONLY,0666) ;

#if	F_DEBUG
	        if (g.debuglevel > 1)
	            eprintf("watch: after possible SRVTAB open, sfd=%d\n",
	                sfd) ;
#endif

	        if (sfd >= 0) {

	            if ((fstat(sfd,&ssb) >= 0) && (ssb.st_mtime > t_srvtab)) {

	                if (g.f.srvtab)
	                    logprintf(&g.lh,
	                        "%s the service file changed\n",
	                        timestr_log(ssb.st_mtime,timebuf)) ;

	                else
	                    logprintf(&g.lh,
	                        "%s the service file has returned\n",
	                        timestr_log(ssb.st_mtime,timebuf)) ;

	                time(&daytime) ;

	                t_srvtab = ssb.st_mtime ;
	                while ((fstat(sfd,&ssb) >= 0) &&
	                    ((daytime - ssb.st_mtime) < SRVIDLETIME)) {

	                    sleep(1) ;

	                    time(&daytime) ;

	                } /* end while */

	                g.f.srvtab = FALSE ;
	                if (u_access(g.srvtab,R_OK) >= 0) {

	                    srvfree(slp) ;

	                    if ((rs = srvinit(slp,g.srvtab)) >= 0) {

	                        g.f.srvtab = TRUE ;
	                        if (t_dir > 1) t_dir -= 1 ;

	                    }

	                    time(&daytime) ;

	                    logprintf(&g.lh,
	                        "%s the new service file is %s\n",
	                        timestr_log(daytime,timebuf),
	                        (g.f.srvtab) ? "OK" : "BAD") ;

	                } else {

	                    logprintf(&g.lh,
	                        "%s the new service file is not accessible\n",
	                        timestr_log(ssb.st_mtime,timebuf)) ;

	                    close(sfd) ;

	                    sfd = -1 ;

	                } /* end if (accessibility of the new one) */

	            } /* end if (service file changed) */

	        } else {

	            if (g.f.srvtab) {

	                g.f.srvtab = FALSE ;
	                logprintf(&g.lh,
	                    "%s service file went away\n",
	                    timestr_log(ssb.st_mtime,timebuf)) ;

	                close(sfd) ;

	                sfd = -1 ;

	            }

	        } /* end if (service file present or not) */

	    } /* end if (we're supposed to have a service table file) */


/* is our directory even there ? */

#if	F_DEBUG
	    if (g.debuglevel > 1)
	        eprintf("watch: before checking directory\n") ;
#endif

	    time(&daytime) ;

	    dfd = dirtest(dfd,g.directory,&dsb) ;

	    if ((dfd >= 0) &&
	        (((daytime - dsb.st_mtime) > (sleeptime * 5)) ||
	        (dsb.st_mtime > t_dir))) {

	        close(dfd) ;

	        dfd = dirtest(-1,g.directory,&dsb) ;

	    } /* end if */

	    if (dfd >= 0) {

#if	F_DEBUG
	        if (g.debuglevel > 1)
	            eprintf("watch: directory seems to be there\n") ;
#endif

	        if (! f_directory) {

	            time(&daytime) ;

	            f_directory = TRUE ;
	            logprintf(&g.lh,
	                "%s directory came back\n",
	                timestr_log(daytime,timebuf)) ;

	            sleeptime = g.polltime ;

	        }

	    } else {

#if	F_DEBUG
	        if (g.debuglevel > 1)
	            eprintf("watch: directory gone ?\n") ;
#endif

	        if (f_directory) {

	            time(&daytime) ;

	            f_directory = FALSE ;
	            logprintf(&g.lh,
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
	        ((nwatch > 0) || (nstale > 0) || (dsb.st_mtime > t_dir))) {


	        close(dfd) ;

	        dfd = -1 ;

#if	F_DEBUG
	        if (g.debuglevel > 1) {

	            gettimeofday(&tod,NULL) ;

	            if (dsb.st_mtime > t_dir) {

	                eprintf("watch: %s dir changed\n",
	                    timestr_ulog(&tod,timebuf)) ;

	                eprintf("watch: dirstat=%s dirtime=%s\n",
	                    timestr_log(dsb.st_mtime,timebuf),
	                    timestr_log(t_dir,timebuf2)) ;

	            } else if (nwatch > 0)
	                eprintf("watch: DIR nwatch=%d\n",nwatch) ;

	            else
	                eprintf("watch: DIR don't know\n") ;

	        }
#endif /* F_DEBUG */

	        t_dir = dsb.st_mtime ;
	        if ((dirp = opendir(g.directory)) != NULL) {

			int	nde = 0 ;


#if	F_DEBUG
	                if (g.debuglevel > 1)
	                    eprintf("watch: opened dir\n") ;
#endif

	            while ((rs = readdir_r(dirp,&de.entry,&dep)) != NULL) {

			nde += 1 ;

#if	F_DEBUG
	                if (g.debuglevel > 1)
	                    eprintf("watch: top of inner while, entry=\"%s\"\n",
	                        dep->d_name) ;
#endif

	                dnp = dep->d_name ;

/* do not play with the "special" entries ! (this is a fast way of deciding) */

	                if (dnp[0] == '.') {

	                    if (dnp[1] == '\0') continue ;

	                    if ((dnp[1] == '.') && (dnp[2] == '\0'))
	                        continue ;

	                }

#if	F_DEBUG
	                if (g.debuglevel > 1)
	                    eprintf("watch: about to search for entry=\"%s\"\n",
	                        dep->d_name) ;
#endif

	                sprintf(tmpfname,"%s/%s",g.directory,dnp) ;

	                if (stat(tmpfname,&jsb) < 0) {

	                    unlink(tmpfname) ;

	                    continue ;
	                }

#if	F_DEBUG
	                if (g.debuglevel > 1)
	                    eprintf("watch: good stat on file\n") ;
#endif

/* do we already have this entry in our table list ? */

	                if ((i = job_search(jlp,dnp,&jep)) >= 0) {

/* yes */

#if	F_DEBUG
	                    if (g.debuglevel > 1)
	                        eprintf("watch: found job \"%s\"\n",dnp) ;
#endif

	                    if (jep->state == STATE_WATCH) {

	                        (void) time(&daytime) ;

	                        if ((jsb.st_size > jep->size) ||
	                            (jsb.st_mtime > jep->mtime)) {

#if	F_DEBUG
	                            if (g.debuglevel > 1)
	                                eprintf("watch: file changed\n") ;
#endif

	                            jep->size = jsb.st_size ;
	                            jep->mtime = jsb.st_mtime ;

	                        } else if (daytime > 
	                            (jep->mtime + JOBIDLETIME)) {

/* we have a new idle file */

#if	F_DEBUG
	                            if (g.debuglevel > 1) eprintf(
	                                "watch: file has not changed\n") ;
#endif

			if (nstarted < maxjobs) {

/* we want to "start" the job */

	                            logsetid(&g.lh,jep->logid) ;

	                            logprintf(&g.lh,"%s processing job\n",
	                                timestr_log(daytime,timebuf)) ;

#if	F_DEBUG
	                            if (g.debuglevel > 1)
	                                eprintf("watch: about to start job\n") ;
#endif

	                            if ((rs = job_start(jep,&jsb,slp)) >= 0) {

#if	F_DEBUG
	                                if (g.debuglevel > 1)
	                                    eprintf("watch: started job\n") ;
#endif

	                                jep->state = STATE_STARTED ;
	                                nstarted += 1 ;

	                                logprintf(&g.lh,"job started OK\n") ;

	                            } else {

	                                sprintf(tmpfname,
	                                    "%s/%s",g.directory,
	                                    jep->filename) ;

	                                unlink(tmpfname) ;

	                                vecelemdel(jlp,i) ;

	                                logprintf(&g.lh,
	                                    "job failed to start, rs=%d\n",
	                                    rs) ;

	                            } /* end if (job started or not) */

			} else {

/* we have to make this job wait since the maximum jobs are already running */

				jep->state = STATE_WAIT ;
				nwaiting += 1 ;

	                            logsetid(&g.lh,jep->logid) ;

	                                logprintf(&g.lh,
	                                    "%s job has to wait\n",
	                                timestr_log(daytime,timebuf)) ;

			} /* end if (want to start or not) */

	                            logsetid(&g.lh,g.logid) ;

	                            nwatch -= 1 ;

	                        } /* end if (file changing or idle) */

	                    } else if ((jep->state == STATE_STALE) &&
	                        (t_srvtab > jep->mtime)) {

/* can we get rid of any old stale jobs ? */

	                        if ((g.f.srvtab && 
	                            ((rs = srvsearch(slp,dnp,&sep)) >= 0)) ||
	                            (g.command != NULL)) {

	                            logsetid(&g.lh,jep->logid) ;

	                            logprintf(&g.lh, "%s processing job\n",
	                                timestr_log(daytime,timebuf)) ;

#if	F_DEBUG
	                            if (g.debuglevel > 1)
	                                eprintf("watch: about to start job\n") ;
#endif

	                            if (job_start(jep,&jsb,slp) >= 0) {

#if	F_DEBUG
	                                if (g.debuglevel > 1)
	                                    eprintf("watch: started job\n") ;
#endif

	                                jep->state = STATE_STARTED ;
	                                nstarted += 1 ;
	                                nstale -= 1 ;

	                            } /* end if */

	                        } /* end if (we have a service for this job) */

	                    } /* end if (handling existing jobs) */

	                } else {

/* no */
/* does this filename match any service entry that we have ? */

#if	F_DEBUG
	                    if (g.debuglevel > 1)
	                        eprintf("watch: new job\n") ;
#endif

	                    strcpy(je.filename,dnp) ;

	                    makejobid(&jobid,&je.logid) ;

#if	F_DEBUG
	                    if (g.debuglevel > 1)
	                        eprintf("watch: jobid=%s\n",je.logid) ;
#endif

	                    je.size = jsb.st_size ;
	                    je.mtime = jsb.st_mtime ;

	                    (void) time(&daytime) ;

	                    logsetid(&g.lh,je.logid) ;

#if	F_DEBUG
	                    if (g.debuglevel > 1)
	                        eprintf("watch: set logid\n") ;
#endif

	                    logprintf(&g.lh, "%s job entering\n",
	                        timestr_log(daytime,timebuf)) ;

#if	F_DEBUG
	                    if (g.debuglevel > 1)
	                        eprintf("watch: made log entry\n") ;
#endif

	                    logprintf(&g.lh,
	                        "f=%s\n",je.filename) ;

	                    rs = BAD ;
	                    if ((g.f.srvtab && 
	                        ((rs = srvsearch(slp,dnp,&sep)) >= 0)) ||
	                        (g.command != NULL)) {

#if	F_DEBUG
	                        if (g.debuglevel > 1) {

	                            if (rs >= 0) eprintf(
	                                "watch: found service=\"%s\"\n",
	                                sep->service) ;

	                            else
	                                eprintf("watch: found command\n") ;

	                        }
#endif /* F_DEBUG */

	                        je.state = STATE_WATCH ;
	                        nwatch += 1 ;
	                        if ((nwatch == 1) && (sleeptime > 5)) {

	                            sleeptime = (sleeptime / 5) ;
	                            if (sleeptime < 5) sleeptime = 5 ;

	                        } /* end if (sleeptime manipulation) */

	                    } else {

	                        je.state = STATE_STALE ;
	                        nstale += 1 ;

#if	F_DEBUG
	                        if (g.debuglevel > 1)
	                            eprintf("watch: did not find service\n") ;
#endif

	                        logprintf(&g.lh,
	                            "stale job\n") ;

	                    } /* end if */

	                    vecelemadd(jlp,&je,sizeof(struct jobentry)) ;

#if	F_DEBUG
	                    if (g.debuglevel > 1)
	                        eprintf("watch: added job to job table\n") ;
#endif

	                } /* end if (new or old job) */

	                    logsetid(&g.lh,g.logid) ;

	            } /* end while (looping through directory entries) */

#if	F_DEBUG
	                    if (g.debuglevel > 1)
	                        eprintf("watch: nde=%d\n",nde) ;
#endif

	            closedir(dirp) ;

	        } /* end if (opened directory) */

	    } /* end if (scanning directory) */


/* scan for any completed jobs */

	    if (nstarted > 0) {

#if	F_DEBUG
	        if (g.debuglevel > 1)
	            eprintf("watch: scanning for completed jobs\n") ;
#endif

	        while ((pid = waitpid(-1,&child_stat,W_OPTIONS)) > 0) {

	            if ((i = job_findpid(jlp,pid,&jep)) >= 0) {

	                (void) time(&daytime) ;

	                logsetid(&g.lh,jep->logid) ;

	                logprintf(&g.lh, "%s job completed, es=%d\n",
	                    timestr_log(daytime,timebuf),
	                    child_stat & 255) ;

	                job_end(jep,child_stat) ;

	                logsetid(&g.lh,g.logid) ;

	                sprintf(tmpfname,"%s/%s",g.directory,jep->filename) ;

	                unlink(tmpfname) ;

	                vecelemdel(jlp,i) ;

	                nstarted -= 1 ;

	            } /* end if */

	        } /* end while (processing job completions) */

	    } /* end if (scanning for completed jobs) */


/* scan to see if any waiting jobs can be started */

	if ((nwaiting > 0) && (nstarted < maxjobs)) {

/* get a job to start */

		for (i = 0 ; (rs = job_get(jlp,i,&jep)) >= 0 ; i += 1) {

			if (jep == NULL) continue ;

			if (jep->state == STATE_WAIT) break ;

		} /* end for */

/* do it (if we have one) */

		if (rs >= 0) {

	                            logsetid(&g.lh,jep->logid) ;

	                            logprintf(&g.lh,"%s processing job (2)\n",
	                                timestr_log(daytime,timebuf)) ;

#if	F_DEBUG
	                            if (g.debuglevel > 1)
	                                eprintf("watch: about to start job\n") ;
#endif

	                            if ((rs = job_start(jep,&jsb,slp)) >= 0) {

#if	F_DEBUG
	                                if (g.debuglevel > 1)
	                                    eprintf("watch: started job\n") ;
#endif

	                                jep->state = STATE_STARTED ;
	                                nstarted += 1 ;

	                                logprintf(&g.lh,"job started OK\n") ;

	                            } else {

	                                sprintf(tmpfname,
	                                    "%s/%s",g.directory,
	                                    jep->filename) ;

	                                unlink(tmpfname) ;

	                                vecelemdel(jlp,i) ;

	                                logprintf(&g.lh,
	                                    "job failed to start, rs=%d\n",
	                                    rs) ;

	                            } /* end if (job started or not) */

	                logsetid(&g.lh,g.logid) ;

			nwaiting -= 1 ;

		} /* end if */

	} /* end if (starting up waiting jobs) */


#if	F_DEBUG
	        if (g.debuglevel > 1) {

	            gettimeofday(&tod,NULL) ;

	            eprintf("watch: %s nstarted=%d nstale=%d nwaiting=%d\n",
	                timestr_ulog(&tod,timebuf),
			nstarted,
			nstale,
			nwaiting) ;

	        }
#endif


/* scan to see if the stale jobs can be started */

#ifdef	F_STALE
	    if ((nstale > 0) && g.f.srvtab && f_srvtab) {

#if	F_DEBUG
	        if (g.debuglevel > 1)
	            eprintf("watch: checking the stales\n") ;
#endif

	        for (i = 0 ; (rs = job_get(jlp,i,&jep)) >= 0 ; i += 1) {

	            if (jep == NULL) continue ;

	            if (jep->state != STATE_STALE) continue ;

#if	F_DEBUG
	            if (g.debuglevel > 1)
	                eprintf("watch: stale=%s\n",jep->filename) ;
#endif

	            sprintf(tmpfname,"%s/%s",g.directory,jep->filename) ;

	            if (u_access(tmpfname,R_OK) < 0) continue ;

	            if (srvsearch(slp,jep->filename,&sep) >= 0) {

	                logsetid(&g.lh,jep->logid) ;

	                logprintf(&g.lh, "%s processing old job\n",
	                    timestr_log(daytime,timebuf)) ;

#if	F_DEBUG
	                if (g.debuglevel > 1)
	                    eprintf("watch: about to start job\n") ;
#endif

	                if (job_start(jep,&jsb,slp) >= 0) {

#if	F_DEBUG
	                    if (g.debuglevel > 1)
	                        eprintf("watch: started job\n") ;
#endif

	                    jep->state = STATE_STARTED ;
	                    nstarted += 1 ;
	                    nstale -= 1 ;

	                } /* end if */

	                logsetid(&g.lh,g.logid) ;

	            } /* end if */

	        } /* end for */

	    } /* end if */
#endif /* F_STALE */


/* maintenance the lock file */

	    if ((g.lockfp != NULL) && 
	        ((daytime - t_lockcheck) >= (MAXSLEEPTIME - 1))) {

	        if ((rs = checklockfile(g.lockfp,g.lockfile,daytime)) != 0) {

	            logprintf(&g.lh,
	                "%s lost my lock file, other PID=%d\n",
	                timestr_log(daytime,timebuf),
	                rs) ;

	            goto badlockfile ;
	        }

	        t_lockcheck = daytime ;

	    } /* end if (maintenancing the lock file) */


/* maintenance the PID mutex lock file */

	    if ((g.pidfp != NULL) && 
	        ((daytime - t_pidcheck) >= (MAXSLEEPTIME - 1))) {

	        if ((rs = checklockfile(g.pidfp,g.pidfile,daytime)) != 0) {

	            logprintf(&g.lh,
	                "%s lost my PID file, other PID=%d\n",
	                timestr_log(daytime,timebuf),
	                rs) ;

	            goto badpidfile ;
	        }

	        t_pidcheck = daytime ;

	    } /* end if (maintenancing the PID file) */




/* overhead scanning functions */

	    if ((nwatch == 0) && (nstarted == 0) &&
	        (sleeptime < g.polltime)) sleeptime += 5 ;


/* wait for a specified amount of time */

	    if (f_interrupt) {

#if	F_DEBUG
	        if (g.debuglevel > 1) {

	            gettimeofday(&tod,NULL) ;

	            eprintf("watch: %s about to wait, sleeptime=%d\n",
	                timestr_ulog(&tod,timebuf),sleeptime) ;
	        }
#endif

	        polltimeout = sleeptime * 1000 ;

	        if ((rs = poll(fds,1,polltimeout)) < 0)
	            rs = (- errno) ;

	        if (rs > 0) {

	            char	eventbuf[80] ;

#if	F_DEBUG
	            if (g.debuglevel > 1) {

	                gettimeofday(&tod,NULL) ;

	                eprintf("watch: %s got an interrupt\n",
	                    timestr_ulog(&tod,timebuf)) ;

	                eprintf("watch: revents %s\n",
	                    reventstr(fds[0].revents,eventbuf)) ;

	            }
#endif /* F_DEBUG */

	            if ((rs = read(ifd,tmpfname,MAXPATHLEN)) < 0)
	                rs = (- errno) ;

#if	F_DEBUG
	            if (g.debuglevel > 1) {

	                gettimeofday(&tod,NULL) ;

	                eprintf("watch: %s back from read, rs=%d\n",
	                    timestr_ulog(&tod,timebuf),rs) ;

	            }

	        } else {

	            if (g.debuglevel > 1) {

	                gettimeofday(&tod,NULL) ;

	                eprintf("watch: %s back from poll, rs=%d\n",
	                    timestr_ulog(&tod,timebuf),rs) ;

	            }

	            sleep(4) ;
#endif /* F_DEBUG */

	        } /* end if (poll) */

	    } else
	        sleep(sleeptime) ;

#if	F_DEBUG
	    if (g.debuglevel > 1) {

	        gettimeofday(&tod,NULL) ;

	        eprintf("watch: %s we are starting a scan cycle\n",
	            timestr_ulog(&tod,timebuf)) ;
	    }
#endif


	} /* end while (outer polling loop) */


	time(&daytime) ;

	logprintf(&g.lh,"%s program exiting\n",
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


/* make a job ID for loggin purposes */
void makejobid(ip,logid)
int	*ip ;
char	logid[15] ;
{
	char	buf[256] ;


	sprintf(buf,"%d.%d",g.pid,(*ip)++) ;

	strwcpy(logid,buf,14) ;

}
/* end subroutine (makejobid) */


#ifdef	COMMENT

static void internal(filename)
char	filename[] ;
{
	int	i, srs = FALSE ;

	char	*cp ;


	if (((cp = strchr(filename,'.')) != NULL) &&
	    isupper(filename) &&
	    (strcmp(cp + 1,INTERNALSUFFIX) == 0) &&
	    ((i = optmatch(funtab,filename,cp - filename)) >= 0)) {

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

#endif


static int dirtest(dfd,dirname,sbp)
int		dfd ;
char		dirname[] ;
struct stat	*sbp ;
{
	int	rs ;


	if (dfd < 0)
	    dfd = u_open(dirname,O_RDONLY,0666) ;

	if (dfd >= 0) {

	if ((rs = u_fstat(dfd,sbp)) >= 0)
		return dfd ;

	close(dfd) ;

	} else
		rs = dfd ;

	return rs ;
}
/* end subroutine (dirtest) */


static void int_term(sn)
int	sn ;
{


	g.f_exit = TRUE ;
}




