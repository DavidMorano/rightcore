/* watch_dir */

/* watch the specified directory */
/* version %I% last modified %G% */


#define	F_DEBUG		1
#define	F_STALE		0


/* revision history :

	= 1991-09-10, Dave Morano

	This program was originally written.


*/


/*****************************************************************************

	This subroutine is responsible for watching the specified
	directory.

	Arguments:

	- slp	service table list pointer
	- jlp	job table list pointer

	Returns:

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

#include	"misc.h"
#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"srvfile.h"



/* local defines */

#define	W_OPTIONS	(WUNTRACED | WNOHANG)



/* external subroutines */

extern int	job_search(), job_start(), job_findpid(), job_end() ;
extern int	checklockfile() ;

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






void watch_dir(slp,jlp,maxjobs)
vecelem	*slp ;
vecelem	*jlp ;
int	maxjobs ;
{
	DIR		*dirp ;

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

}
/* end if (watch_dir) */



