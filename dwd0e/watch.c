/* watch */

/* watch the specified directory */
/* version %I% last modified %G% */


#define	F_DEBUG		1
#define	F_STALE		0
#define	F_NODOTS	1


/* revision history :

	= 91/09/01, David Morano

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
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stropts.h>
#include	<poll.h>
#include	<dirent.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<vecelem.h>
#include	<vecstr.h>
#include	<fsdir.h>
#include	<lfm.h>

#include	"misc.h"
#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"job.h"
#include	"srvtab.h"



/* local defines */

#define	W_OPTIONS	(WUNTRACED | WNOHANG)
#define	TI_PIDCHECK	(MAXSLEEPTIME - 1)



/* external subroutines */

extern int	checklockfile(struct proginfo *,bfile *,char *,char *,
				time_t,pid_t) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timevalstr_ulog(struct timeval *,char *) ;
extern char	*strwcpy(char *,char *,int) ;


/* external variables */


/* local global variables */

static int	f_exit = FALSE ;


/* local structures */


/* forwards references */

static int	dirtest(int,char *,struct ustat *) ;

static void	makejobid() ;
static void	int_term() ;


/* local data */

/* built-in function table */

#define	FUN_REPORT	0
#define	FUN_EXIT	1


static const char	*funtab[] = {
	"REPORT",
	"EXIT",
	NULL
} ;





int watch(pip,slp,jlp,maxjobs,filetime)
struct proginfo	*pip ;
SRVTAB		*slp ;
JOB		*jlp ;
int		maxjobs, filetime ;
{
	fsdir		dir ;

	fsdir_slot	de ;

	struct dirent	*dep ;

	struct pollfd	fds[1] ;

	struct ustat	ssb, dsb, jsb ;

	struct timeval	tod ;

	struct sigaction	sigs ;

	struct jobentry		je, *jep ;

	SRVTAB_ENTRY	*sep ;

	pid_t		pid ;

	sigset_t	signalmask ;

	time_t		daytime ;
	time_t		tim_srvtab = 1 ;
	time_t		tim_dir = 1 ;
	time_t		tim_check = 1 ;
	time_t		tim_lockcheck = 1 ;
	time_t		tim_pidcheck = 1 ;
	time_t		tim_start = 0 ;
	time_t		tim_run = 0 ;

	int	rs, i ;
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
	int	f_interrupt = pip->f.interrupt ;
	int	f_srvtab = pip->f.srvtab ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[50], timebuf2[50] ;
	char	*dnp ;


#if	F_DEBUG
	if (pip->debuglevel >= 4)
	    eprintf("watch: entered, filetime=%d\n",filetime) ;
#endif

	if (pip->polltime > MAXSLEEPTIME)
	    pip->polltime = MAXSLEEPTIME ;

	sleeptime = pip->polltime ;

/* let's try to cache the directory FD */

	f_directory = FALSE ;
	if ((dfd = dirtest(-1,pip->directory,&dsb)) >= 0)
	    f_directory = TRUE ;

	else
	    logfile_printf(&pip->lh,"queue directory not accessible\n") ;


/* what about an interrupt file ? */

	if (pip->f.interrupt &&
	    ((ifd = u_open(pip->interrupt,O_RDWR,0664)) >= 0)) {

	    uc_closeonexec(ifd) ;

	    fds[0].fd = ifd ;
	    fds[0].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;

	} else
	    pip->f.interrupt = FALSE ;

	f_interrupt = pip->f.interrupt ;


/* what about the service table */

	sfd = -1 ;
	if (pip->f.srvtab) {

	    if (u_access(pip->srvtab,R_OK) >= 0)
	        sfd = u_open(pip->srvtab,O_RDONLY,0666) ;

	    tim_srvtab = 1 ;
	    if ((sfd >= 0) && (u_fstat(sfd,&ssb) >= 0))
	        tim_srvtab = ssb.st_mtime ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	        eprintf("watch: starting sfd=%d\n",sfd) ;
#endif

	} /* end if (we have a SRVTAB file) */


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_term ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGTERM,&sigs,NULL) ;



/* let's go ! */

	u_time(&tim_start) ;

	f_exit = FALSE ;
	while (! f_exit) {


/* has the service table file changed since we last looked ? */

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	        eprintf("watch: before srvtab, nwatch=%d\n",nwatch) ;
#endif

	    if (f_srvtab) {

#if	F_DEBUG
	if (pip->debuglevel >= 4) {

	            gettimeofday(&tod,NULL) ;

	            eprintf("watch: %s inside srvtab if, sfd=%d\n",
	                timevalstr_ulog(&tod,timebuf),sfd) ;

	        }
#endif /* F_DEBUG */

	        if (sfd < 0)
	            sfd = u_open(pip->srvtab,O_RDONLY,0666) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	            eprintf("watch: after possible SRVTAB open, sfd=%d\n",
	                sfd) ;
#endif

	        if (sfd >= 0) {

	            if ((u_fstat(sfd,&ssb) >= 0) && 
				(ssb.st_mtime > tim_srvtab)) {

	                if (pip->f.srvtab)
	                    logfile_printf(&pip->lh,
	                        "%s the service file changed\n",
	                        timestr_log(ssb.st_mtime,timebuf)) ;

	                else
	                    logfile_printf(&pip->lh,
	                        "%s the service file has returned\n",
	                        timestr_log(ssb.st_mtime,timebuf)) ;

	                u_time(&daytime) ;

	                tim_srvtab = ssb.st_mtime ;
	                while ((u_fstat(sfd,&ssb) >= 0) &&
	                    ((daytime - ssb.st_mtime) < SRVIDLETIME)) {

	                    sleep(1) ;

	                    u_time(&daytime) ;

	                } /* end while */

	                pip->f.srvtab = FALSE ;
	                if (u_access(pip->srvtab,R_OK) >= 0) {

	                    (void) srvtab_free(slp) ;

	                    rs = srvtab_init(slp,pip->srvtab,NULL) ;

				if (rs >= 0) {

	                        pip->f.srvtab = TRUE ;
	                        if (tim_dir > 1) 
					tim_dir -= 1 ;

	                    }

	                    u_time(&daytime) ;

	                    logfile_printf(&pip->lh,
	                        "%s the new service file is %s\n",
	                        timestr_log(daytime,timebuf),
	                        (pip->f.srvtab) ? "OK" : "BAD") ;

	                } else {

	                    logfile_printf(&pip->lh,
	                        "%s the new service file is not accessible\n",
	                        timestr_log(ssb.st_mtime,timebuf)) ;

	                    u_close(sfd) ;

	                    sfd = -1 ;

	                } /* end if (accessibility of the new one) */

	            } /* end if (service file changed) */

	        } else {

	            if (pip->f.srvtab) {

	                pip->f.srvtab = FALSE ;
	                logfile_printf(&pip->lh,
	                    "%s service file went away\n",
	                    timestr_log(ssb.st_mtime,timebuf)) ;

	                u_close(sfd) ;

	                sfd = -1 ;

	            }

	        } /* end if (service file present or not) */

	    } /* end if (we're supposed to have a service table file) */


/* is our directory even there ? */

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	        eprintf("watch: before checking directory\n") ;
#endif

	    u_time(&daytime) ;

	    dfd = dirtest(dfd,pip->directory,&dsb) ;

	    if ((dfd >= 0) &&
	        (((daytime - tim_check) > (sleeptime * 50)) ||
	        (dsb.st_mtime > tim_dir))) {

		tim_check = daytime ;
	        u_close(dfd) ;

	        dfd = dirtest(-1,pip->directory,&dsb) ;

	    } /* end if */

	    if (dfd >= 0) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	            eprintf("watch: directory seems to be there\n") ;
#endif

	        if (! f_directory) {

	            u_time(&daytime) ;

	            f_directory = TRUE ;
	            logfile_printf(&pip->lh,
	                "%s directory came back\n",
	                timestr_log(daytime,timebuf)) ;

	            sleeptime = pip->polltime ;

	        } /* end if */

	    } else {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	            eprintf("watch: directory gone ?\n") ;
#endif

	        if (f_directory) {

	            u_time(&daytime) ;

	            f_directory = FALSE ;
	            logfile_printf(&pip->lh,
	                "%s directory went away\n",
	                timestr_log(daytime,timebuf)) ;

	        }

	        sleeptime = (sleeptime * 2) ;
	        if (sleeptime > (10 * pip->polltime))
	            sleeptime = (10 * pip->polltime) ;

	        if (sleeptime > MAXSLEEPTIME)
	            sleeptime = MAXSLEEPTIME ;

	    } /* end if (directory test) */


/* has the directory modification time changed ? */

#if	F_DEBUG
	if (pip->debuglevel >= 4) {

	        eprintf("watch: about to decide about looking\n") ;

	        eprintf("watch: timdir=%s\n",
	                    timestr_log(tim_dir,timebuf2)) ;

	        eprintf("watch: dir_mtime=%s\n",
	                    timestr_log(dsb.st_mtime,timebuf)) ;

	}
#endif /* F_DEBUG */

	    if ((dfd >= 0) && 
	        ((nwatch > 0) || (nstale > 0) || (dsb.st_mtime > tim_dir))) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	            eprintf("watch: looking at dir\n") ;
#endif

	        u_close(dfd) ;

	        dfd = -1 ;

#if	F_DEBUG
	if (pip->debuglevel >= 4) {

	            gettimeofday(&tod,NULL) ;

	            if (dsb.st_mtime > tim_dir) {

	                eprintf("watch: dir changed %s\n",
	                    timevalstr_ulog(&tod,timebuf)) ;

	                eprintf("watch: dir_mtime=%s timdir=%s\n",
	                    timestr_log(dsb.st_mtime,timebuf),
	                    timestr_log(tim_dir,timebuf2)) ;

	            } else if (nwatch > 0)
	                eprintf("watch: DIR nwatch=%d\n",nwatch) ;

	            else
	                eprintf("watch: DIR don't know\n") ;

	        }
#endif /* F_DEBUG */

	        tim_dir = dsb.st_mtime ;
		if (fsdir_open(&dir,pip->directory) >= 0) {

	            int	nde = 0 ;


#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                eprintf("watch: opened dir\n") ;
#endif

	            while (fsdir_read(&dir,&de) > 0) {

			dep = &de.entry ;
	                nde += 1 ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
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

#if	F_NODOTS
		if (dnp[0] == '.') continue ;
#endif /* F_NODOTS */

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                    eprintf("watch: about to search for entry=\"%s\"\n",
	                        dep->d_name) ;
#endif

	                sprintf(tmpfname,"%s/%s",pip->directory,dnp) ;

	                if (u_stat(tmpfname,&jsb) < 0) {

	                    u_unlink(tmpfname) ;

	                    continue ;
	                }

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                    eprintf("watch: good stat on file\n") ;
#endif

/* do we already have this entry in our table list ? */

	                if ((i = job_search(jlp,pip,dnp,&jep)) >= 0) {

/* yes */

#if	F_DEBUG
	if (pip->debuglevel >= 4) {
	                        eprintf("watch: found job \"%s\"\n",
					dnp) ;
	                        eprintf("watch: job state=%d\n",jep->state) ;
	                        eprintf("watch: job mtime=%s\n",
					timestr_log(jsb.st_mtime,timebuf)) ;
			}
#endif /* F_DEBUG */

	                    if (jep->state == STATE_WATCH) {

	                        u_time(&daytime) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4) {
	                                eprintf("watch: job being watched\n") ;
	                                eprintf("watch: filetime=%d\n",
						filetime) ;
	                                eprintf("watch: daytime %s\n",
						timestr_log(daytime,timebuf)) ;
					}
#endif

	                        if ((jsb.st_size > jep->size) ||
	                            (jsb.st_mtime > jep->mtime)) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                                eprintf("watch: file changed\n") ;
#endif

	                            jep->size = jsb.st_size ;
	                            jep->mtime = jsb.st_mtime ;

	                        } else if ((daytime - je.daytime) > 
	                            (jsb.st_mtime - je.stime + filetime)) {

/* we have a new idle file */

#if	F_DEBUG
	if (pip->debuglevel >= 4)
					eprintf(
					    "watch: file has not changed\n") ;
#endif

	                            if (nstarted < maxjobs) {

/* we want to "start" the job */

	                                logfile_setid(&pip->lh,jep->logid) ;

	                                logfile_printf(&pip->lh,
						"%s processing job\n",
	                                    timestr_log(daytime,timebuf)) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                                    eprintf("watch: start job\n") ;
#endif

	                                if ((rs = job_start(jlp,jep,pip,
						&jsb,slp)) >= 0) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                                        eprintf("watch: started\n") ;
#endif

	                                    jep->state = STATE_STARTED ;
	                                    nstarted += 1 ;

	                                    logfile_printf(&pip->lh,
						"started OK\n") ;

	                                } else {

	                                    sprintf(tmpfname,
	                                        "%s/%s",pip->directory,
	                                        jep->filename) ;

	                                    u_unlink(tmpfname) ;

	                                    vecelem_del(jlp,i) ;

	                                    logfile_printf(&pip->lh,
	                                        "job failed to start, rs=%d\n",
	                                        rs) ;

	                                } /* end if (job started or not) */

	                            } else {

/* we have to make this job wait since the maximum jobs are already running */

	                                jep->state = STATE_WAIT ;
	                                nwaiting += 1 ;

	                                logfile_setid(&pip->lh,jep->logid) ;

	                                logfile_printf(&pip->lh,
	                                    "%s job has to wait\n",
	                                    timestr_log(daytime,timebuf)) ;

	                            } /* end if (want to start or not) */

	                            logfile_setid(&pip->lh,pip->logid) ;

	                            nwatch -= 1 ;

	                        } /* end if (file changing or idle) */

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                                eprintf(
					    "watch: done w/ changing/idle\n") ;
#endif

	                    } else if ((jep->state == STATE_STALE) &&
	                        (tim_srvtab > jep->mtime)) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                                eprintf("watch: job stale ?\n") ;
#endif

/* can we get rid of any old stale jobs ? */

	                        if ((pip->f.srvtab && 
	                            ((rs = srvtab_match(slp,dnp,&sep)) >= 0)) ||
	                            (pip->command != NULL)) {

	                            logfile_setid(&pip->lh,jep->logid) ;

	                            logfile_printf(&pip->lh, 
					"%s processing job\n",
	                                timestr_log(daytime,timebuf)) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                                eprintf("watch: about to start job\n") ;
#endif

	                            if (job_start(jlp,jep,pip,&jsb,slp) >= 0) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                                    eprintf("watch: started job\n") ;
#endif

	                                jep->state = STATE_STARTED ;
	                                nstarted += 1 ;
	                                nstale -= 1 ;

	                            } /* end if */

	                        } /* end if (we have a service for this job) */

	                    } /* end if (handling existing jobs) */

	                } else {

/* no, new job */
/* does this filename match any service entry that we have ? */

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                        eprintf("watch: new job\n") ;
#endif

	                    strcpy(je.filename,dnp) ;

	                    makejobid(pip,&jobid,&je.logid) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                        eprintf("watch: jobid=%s\n",je.logid) ;
#endif

	                    je.size = jsb.st_size ;
	                    je.mtime = jsb.st_mtime ;
			je.stime = jsb.st_mtime ;
			je.daytime = daytime ;

	                    u_time(&daytime) ;

	                    logfile_setid(&pip->lh,je.logid) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                        eprintf("watch: set logid\n") ;
#endif

	                    logfile_printf(&pip->lh, "%s job entering\n",
	                        timestr_log(daytime,timebuf)) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                        eprintf("watch: made log entry\n") ;
#endif

	                    logfile_printf(&pip->lh,
	                        "job=%s\n",je.filename) ;

	                    rs = BAD ;
	                    if ((pip->f.srvtab && 
	                        ((rs = srvtab_match(slp,dnp,&sep)) >= 0)) ||
	                        (pip->command != NULL)) {

#if	F_DEBUG
	if (pip->debuglevel >= 4) {

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
	if (pip->debuglevel >= 4)
	                            eprintf("watch: did not find service\n") ;
#endif

	                        logfile_printf(&pip->lh,
	                            "stale job\n") ;

	                    } /* end if */

	                    vecelem_add(jlp,&je,sizeof(struct jobentry)) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                        eprintf("watch: added job to job table\n") ;
#endif

	                } /* end if (new or old job) */

	                logfile_setid(&pip->lh,pip->logid) ;

	            } /* end while (looping through directory entries) */

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                eprintf("watch: bottom nde=%d\n",nde) ;
#endif

	            fsdir_close(&dir) ;

	        } /* end if (opened directory) */

	    } /* end if (scanning directory) */


/* scan for any completed jobs */

	    if (nstarted > 0) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	            eprintf("watch: scanning for completed jobs\n") ;
#endif

	        while ((pid = waitpid(-1,&child_stat,W_OPTIONS)) > 0) {

	            if ((i = job_findpid(jlp,pip,pid,&jep)) >= 0) {

			time_t	elapsed ;


	                u_time(&daytime) ;

	                logfile_setid(&pip->lh,jep->logid) ;

	                logfile_printf(&pip->lh, "%s job completed, ex=%d\n",
	                    timestr_log(daytime,timebuf),
	                    child_stat & 255) ;

			elapsed = daytime - jep->daytime ;
	                logfile_printf(&pip->lh, "elapsed time %s\n",
	                    timestr_elapsed(elapsed,timebuf)) ;

	                job_end(jlp,jep,pip,child_stat) ;

	                logfile_setid(&pip->lh,pip->logid) ;

	                sprintf(tmpfname,"%s/%s",pip->directory,jep->filename) ;

	                u_unlink(tmpfname) ;

	                vecelem_del(jlp,i) ;

	                nstarted -= 1 ;

	            } /* end if */

	        } /* end while (processing job completions) */

	    } /* end if (scanning for completed jobs) */


/* scan to see if any waiting jobs can be started */

	    if ((nwaiting > 0) && (nstarted < maxjobs)) {

/* get a job to start */

	        for (i = 0 ; (rs = job_get(jlp,pip,i,&jep)) >= 0 ; i += 1) {

	            if (jep == NULL) continue ;

	            if (jep->state == STATE_WAIT) break ;

	        } /* end for */

/* do it (if we have one) */

	        if (rs >= 0) {

	            logfile_setid(&pip->lh,jep->logid) ;

	            logfile_printf(&pip->lh,"%s processing job (2)\n",
	                timestr_log(daytime,timebuf)) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                eprintf("watch: about to start job\n") ;
#endif

	            if ((rs = job_start(jlp,jep,pip,&jsb,slp)) >= 0) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                    eprintf("watch: started job\n") ;
#endif

	                jep->state = STATE_STARTED ;
	                nstarted += 1 ;

	                logfile_printf(&pip->lh,"job started OK\n") ;

	            } else {

	                sprintf(tmpfname,
	                    "%s/%s",pip->directory,
	                    jep->filename) ;

	                u_unlink(tmpfname) ;

	                vecelem_del(jlp,i) ;

	                logfile_printf(&pip->lh,
	                    "job failed to start, rs=%d\n",
	                    rs) ;

	            } /* end if (job started or not) */

	            logfile_setid(&pip->lh,pip->logid) ;

	            nwaiting -= 1 ;

	        } /* end if */

	    } /* end if (starting up waiting jobs) */


#if	F_DEBUG
	if (pip->debuglevel >= 4) {

	        gettimeofday(&tod,NULL) ;

	        eprintf("watch: %s nstarted=%d nstale=%d nwaiting=%d\n",
	            timevalstr_ulog(&tod,timebuf),
	            nstarted,
	            nstale,
	            nwaiting) ;

	    }
#endif


/* scan to see if the stale jobs can be started */

#ifdef	F_STALE
	    if ((nstale > 0) && pip->f.srvtab && f_srvtab) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	            eprintf("watch: checking the stales\n") ;
#endif

	        for (i = 0 ; (rs = job_get(jlp,pip,i,&jep)) >= 0 ; i += 1) {

	            if (jep == NULL) continue ;

	            if (jep->state != STATE_STALE) continue ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                eprintf("watch: stale=%s\n",jep->filename) ;
#endif

	            sprintf(tmpfname,"%s/%s",pip->directory,jep->filename) ;

	            if (u_access(tmpfname,R_OK) < 0) continue ;

	            if (srvtab_match(slp,jep->filename,&sep) >= 0) {

	                logfile_setid(&pip->lh,jep->logid) ;

	                logfile_printf(&pip->lh, "%s processing old job\n",
	                    timestr_log(daytime,timebuf)) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                    eprintf("watch: about to start job\n") ;
#endif

	                if (job_start(jlp,jep,pip,&jsb,slp) >= 0) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                        eprintf("watch: started job\n") ;
#endif

	                    jep->state = STATE_STARTED ;
	                    nstarted += 1 ;
	                    nstale -= 1 ;

	                } /* end if */

	                logfile_setid(&pip->lh,pip->logid) ;

	            } /* end if */

	        } /* end for */

	    } /* end if */
#endif /* F_STALE */


/* maintenance the lock file */

	    if ((pip->lockfp != NULL) && 
	        ((daytime - tim_lockcheck) >= (MAXSLEEPTIME - 1))) {

	        if ((rs = checklockfile(pip,pip->lockfp,
			pip->lockfname,BANNER,daytime,pip->pid)) != 0) {

	            logfile_printf(&pip->lh,
	                "%s lost my lock file, other PID=%d\n",
	                timestr_log(daytime,timebuf),
	                rs) ;

	            goto badlockfile ;
	        }

	        tim_lockcheck = daytime ;

	    } /* end if (maintenancing the lock file) */


/* maintenance the PID mutex lock file */

	    if ((pip->pidfname != NULL) && 
	        ((daytime - tim_pidcheck) >= TI_PIDCHECK)) {

			LFM_CHECK	ci ;


	        rs = lfm_check(&pip->pider,&ci, daytime) ;

		if (rs < 0) {

#if	F_DEBUG
	if (pip->debuglevel >= 4)
	                    eprintf("watch: lost PID file rs=%d pid=%d\n",
				rs,ci.pid) ;
#endif

	            logfile_printf(&pip->lh,
	                "%s lost my PID file, other pid=%d\n",
	                timestr_log(daytime,timebuf),
	                ci.pid) ;

	            goto badpidfile ;
	        }

	        tim_pidcheck = daytime ;

	    } /* end if (maintenancing the PID file) */


/* overhead scanning functions */

	    if ((nwatch == 0) && (nstarted == 0) &&
	        (sleeptime < pip->polltime)) 
		sleeptime += 5 ;


/* wait for a specified amount of time */

	    if (f_interrupt) {

#if	F_DEBUG
	if (pip->debuglevel >= 4) {

	            gettimeofday(&tod,NULL) ;

	            eprintf("watch: %s about to wait, sleeptime=%d\n",
	                timevalstr_ulog(&tod,timebuf),sleeptime) ;
	        }
#endif /* F_DEBUG */

	        polltimeout = sleeptime * 1000 ;
	        rs = u_poll(fds,1,polltimeout) ;

	        if (rs > 0) {

	            char	eventbuf[80] ;

#if	F_DEBUG
	if (pip->debuglevel >= 4) {

	                gettimeofday(&tod,NULL) ;

	                eprintf("watch: %s got an interrupt\n",
	                    timevalstr_ulog(&tod,timebuf)) ;

	                eprintf("watch: revents %s\n",
	                    d_reventstr(fds[0].revents,eventbuf)) ;

	            }
#endif /* F_DEBUG */

	            rs = u_read(ifd,tmpfname,MAXPATHLEN) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4) {

	                gettimeofday(&tod,NULL) ;

	                eprintf("watch: %s back from read, rs=%d\n",
	                    timevalstr_ulog(&tod,timebuf),rs) ;

	            }
#endif /* F_DEBUG */

	        } /* end if (had something from poll) */

		if (rs < 0) {

#if	F_DEBUG
	if (pip->debuglevel >= 4) {

	                gettimeofday(&tod,NULL) ;

	                eprintf("watch: %s back from poll, rs=%d\n",
	                    timevalstr_ulog(&tod,timebuf),rs) ;

	            }

	            sleep(4) ;
#endif /* F_DEBUG */

	        } /* end if (poll) */

	    } else
	        sleep(sleeptime) ;

#if	F_DEBUG
	if (pip->debuglevel >= 4) {

	        gettimeofday(&tod,NULL) ;

	        eprintf("watch: %s we are starting a scan cycle\n",
	            timevalstr_ulog(&tod,timebuf)) ;
	    }
#endif


/* are we in the special poll-only mode ? */

		u_time(&daytime) ;

		tim_run = daytime - tim_start ;
		if (pip->f.poll && (nwaiting == 0) && (nstarted == 0) &&
			(tim_run > pip->pollmodetime))
			f_exit = TRUE ;



	} /* end while (outer polling loop) */


	u_time(&daytime) ;

	logfile_printf(&pip->lh,"%s program exiting\n",
	    timestr_log(daytime,timebuf)) ;


	if (pip->f.interrupt) 
		u_close(ifd) ;

	if (dfd >= 0) 
		u_close(dfd) ;

	if (sfd >= 0) 
		u_close(sfd) ;

	return OK ;

killed:
badlockfile:
badpidfile:
	if (pip->f.interrupt) 
		u_close(ifd) ;

	if (dfd >= 0) 
		u_close(dfd) ;

	if (sfd >= 0) 
		u_close(sfd) ;

	return BAD ;
}
/* end subroutine (watch) */


/* make a job ID for loging purposes */
void makejobid(pip,ip,logid)
struct proginfo	*pip ;
int	*ip ;
char	logid[LOGIDLEN] ;
{


	bufprintf(logid,LOGIDLEN,"%d.%d",
		pip->pid,(*ip)++) ;

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
/* end subroutine (internal) */

#endif


static int dirtest(dfd,dirname,sbp)
int		dfd ;
char		dirname[] ;
struct ustat	*sbp ;
{
	int	rs ;


	if (dfd < 0) {

	    dfd = u_open(dirname,O_RDONLY,0666) ;

	    uc_closeonexec(dfd) ;

	}

	if (dfd >= 0) {

	    if ((rs = u_fstat(dfd,sbp)) >= 0)
	        return dfd ;

	    u_close(dfd) ;

	} else
	    rs = dfd ;

	return rs ;
}
/* end subroutine (dirtest) */


static void int_term(sn)
int	sn ;
{


	f_exit = TRUE ;
}



