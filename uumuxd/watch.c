/* watch */

/* watch (listen on) the specified socket */
/* version %I% last modified %G% */


#define	CF_DEBUG	1
#define	F_STALE		0


/* revision history:

	= 91/09/10, David A­D­ Morano

	This subroutine was adopted from the DWD program.


*/


/*****************************************************************************

	This subroutine is responsible for listening on the given
	socket and spawning off a program to handle any incoming
	connection.

	This subroutine is called with the folowing arguments :

	gp	global data pointer
	s	socket to listen on
	elp	vector list of exported variables

	Returns:

	OK	doesn't really matter in the current implementation


*****************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/time.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<ftw.h>
#include	<dirent.h>
#include	<limits.h>
#include	<string.h>
#include	<stropts.h>
#include	<poll.h>

#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<baops.h>
#include	<vsystem.h>

#include	"srvtab.h"

#include	"localmisc.h"
#include	"jobdb.h"
#include	"builtin.h"
#include	"srvpe.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	W_OPTIONS	(WNOHANG)
#define	POLLTIMEOUT	2
#define	MAINTTIME	(2 * 60)



/* external subroutines */

extern int	checklockfile() ;
extern int	handle(struct global *,
			SRVPE *,int,int,VECSTR *,SRVTAB *,BUILTIN *) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;

#if	CF_DEBUG
extern char	*d_reventstr() ;
#endif


/* externals variables */

extern struct global	g ;


/* forward references */

static int	writeout() ;

static void	int_all(int) ;


/* local global variables */


/* local structures */


/* local data */






int watch(gp,pbp,s,elp,sfp,bip)
struct global	*gp ;
SRVPE		*pbp ;
int		s ;
VECSTR		*elp ;
SRVTAB		*sfp ;
BUILTIN		*bip ;
{
	struct pollfd	fds[2] ;

	struct sigaction	sigs ;

	struct sockaddr_in	from ;

	struct ustat		sb ;

	JOBDB		jdb ;

	JOBDB_ENT	*jep ;

	pid_t		pid ;

	sigset_t	signalmask ;

	time_t		daytime ;
	time_t		t_lockcheck = 1 ;
	time_t		t_pidcheck = 1 ;

	int	i, rs, len ;
	int	polltimeout = POLLTIMEOUT * 1000 ;
	int	child_stat ;
	int	nfds ;
	int	re ;
	int	fromlen ;
	int	ns ;
	int	serial = 0 ;
	int	efd ;

	char	logid[JOBDB_JOBIDLEN + 1] ;
	char	timebuf[TIMEBUFLEN], timebuf2[TIMEBUFLEN] ;
	char	*cp ;

#if	CF_DEBUG
	char	buf[BUFLEN + 1] ;
#endif


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("watch: entered w/ socket on FD %d\n",s) ;
#endif

	jobdb_init(&jdb,1,gp->tmpdir) ;


/* we want to receive the new socket (from 'accept') above these guys */

	for (i = 0 ; i < 3 ; i += 1) {

	    if (u_fstat(i,&sb) < 0)
	        (void) u_open("/dev/null",O_RDONLY,0666) ;

	}


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


/* let's go ! */

	nfds = 0 ;
	fds[nfds].fd = s ;
	fds[nfds].events = POLLIN | POLLPRI ;
	fds[nfds].revents = 0 ;
	nfds += 1 ;


/* top of loop */
top:

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("watch: about to poll\n") ;
#endif

	rs = u_poll(fds,nfds,polltimeout) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("watch: back from poll w/ rs=%d\n",rs) ;
#endif

	if (rs > 0) {

	    re = fds[0].revents ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("watch: back from poll, re=%s\n",
	            d_reventstr(re,buf,BUFLEN)) ;
#endif

	    if ((re & POLLIN) || (re & POLLPRI)) {

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("watch: got a poll in, normal=%d priority=%d\n",
	                (re & POLLIN) ? 1 : 0,(re & POLLPRI) ? 1 : 0) ;
#endif

	        fromlen = sizeof(struct sockaddr) ;
	        ns = u_accept(s,&from,&fromlen) ;

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("watch: we accepted a call, rs=%d\n",ns) ;
#endif

	        if (ns < 0) {

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf(
	                    "watch: bad return from accept, rs=%d\n",
	                    ns) ;
#endif

	            goto baderr ;
	        }


/* enter this job into the database */

	        bufprintf(logid,JOBDB_JOBIDLEN,"%d.%d",
	            gp->ppid,serial) ;

		logfile_setid(&gp->lh,logid) ;

	        if ((rs = jobdb_newjob(&jdb,logid)) >= 0) {

	            jobdb_get(&jdb,rs,&jep) ;


/* let's fork the processing subroutine and get on with our lives ! */

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf("watch: passing ns=%d\n",ns) ;
#endif

	            pid = uc_fork() ;

	            if (pid == 0) {

/* we are now the CHILD !! */

			gp->logid = jep->jobid ;
	                gp->pid = getpid() ;

	                for (i = 0 ; i < 3 ; i += 1)
	                    u_close(i) ;

	                u_close(s) ;

	                u_dup(ns) ;

	                u_dup(ns) ;

	                u_open(jep->efname,O_WRONLY,0666) ;

	                rs = handle(gp,pbp,ns,ns,elp,sfp,bip) ;

	                logfile_close(&gp->lh) ;

	                if (rs < 0)
	                    u_unlink(jep->efname) ;

	                uc_exit((rs < 0) ? 1 : 0) ;

	            } /* end if */

	            if (pid < 0) {

	                logfile_printf(&gp->lh,
				"we couldn't do a fork (%d)\n",
	                    pid) ;

	            }

			jep->pid = pid ;

	        } else {

	            cp = "- could not allocate job resource" ;
	            uc_writen(ns,cp,strlen(cp)) ;

	            logfile_printf(&gp->lh,"jobdb new failed, rs=%d\n",rs) ;

	        }

	        u_close(ns) ;

			serial += 1 ;
		logfile_setid(&gp->lh,gp->logid) ;

	    } else if (re & POLLHUP)
	        goto badhup ;

	    else if (re & POLLERR)
	        goto baderr ;

	} /* end if (something from 'poll') */


/* are there any completed jobs yet ? */

	if ((rs = u_waitpid(-1,&child_stat,W_OPTIONS)) > 0) {

	    int	ji ;


#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf("watch: child exit, pid=%d stat=%d\n",
				rs,(child_stat & 0xFF)) ;
#endif

	    pid = rs ;
	    if ((ji = jobdb_findpid(&jdb,pid,&jep)) >= 0) {

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf("watch: found child, ji=%d\n",ji) ;
#endif

	        logfile_setid(&gp->lh,jep->jobid) ;

/* process this guy's termination */

	        if ((efd = u_open(jep->efname,O_RDONLY,0666)) >= 0) {

	            rs = SR_OK ;
	            (void) time(&daytime) ;

	            logfile_printf(&g.lh, "%s server exit, es=%d\n",
	                timestr_log(daytime,timebuf),
	                child_stat & 255) ;

	            writeout(gp,efd,"* standard error *") ;

		    logfile_printf(&gp->lh,"elapsed time %s\n",
			timestr_elapsed((daytime - jep->atime),timebuf)) ;

	        } else {

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf("watch: child did not 'exec'\n") ;
#endif

	            logfile_printf(&gp->lh,"server did not 'exec'\n") ;

		}

	        jobdb_del(&jdb,ji) ;

	        logfile_setid(&gp->lh,gp->logid) ;

	    } else
	        logfile_printf(&gp->lh,"unknown PID=%d\n",rs) ;

	} /* end if (a child process exited) */


/* maintenance the lock file */

	(void) time(&daytime) ;

	if ((gp->lockfp != NULL) && 
	    ((daytime - t_lockcheck) >= (MAINTTIME - 1))) {

	    if ((rs = checklockfile(gp->lockfp,gp->lockfile,daytime)) != 0) {

	        logfile_printf(&gp->lh,
	            "%s another program has my lock file, other PID=%d\n",
	            timestr_log(daytime,timebuf),
	            rs) ;

	        goto badlockfile ;
	    }

	    t_lockcheck = daytime ;

	} /* end if (maintaining the lock file) */


/* maintenance the PID mutex lock file */

	if ((gp->pidfp != NULL) && 
	    ((daytime - t_pidcheck) >= (MAINTTIME - 1))) {

	    if ((rs = checklockfile(gp->pidfp,gp->pidfname,daytime)) != 0) {

	        logfile_printf(&gp->lh,
	            "%s another program has my PID file, other PID=%d\n",
	            timestr_log(daytime,timebuf),
	            rs) ;

	        goto badpidfile ;
	    }

	    t_pidcheck = daytime ;

	} /* end if (maintaining the PID mutex file) */


/* check if the server file may have changed */

	if ((rs = srvtab_check(sfp,daytime,NULL)) > 0)
		logfile_printf(&gp->lh,"%s server table file changed\n",
			timestr_log(daytime,timebuf)) ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
		debugprintf("watch: jobs=%d\n",
			jobdb_count(&jdb)) ;
#endif

/* go back to the top of the loop */

	if (! gp->f_exit) goto top ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("watch: returning !\n") ;
#endif


	jobdb_free(&jdb) ;


	(void) time(&daytime) ;

	logfile_printf(&gp->lh,"%s daemon exiting\n",
	    timestr_log(daytime,timebuf)) ;

	return OK ;

/* we got killed , sigh ! */
badlockfile:
badpidfile:

badhup:
baderr:
	jobdb_free(&jdb) ;

	return BAD ;
}
/* end subroutine (watch) */


/* watch just one daemon server program */
int watchone(gp,pbp,ns,elp,sfp,bip)
struct global	*gp ;
SRVPE		*pbp ;
int		ns ;
VECSTR		*elp ;
SRVTAB		*sfp ;
BUILTIN		*bip ;
{
	struct ustat	sb ;

	JOBDB		jdb ;

	JOBDB_ENT	*jep ;

	pid_t		pid ;

	time_t		daytime ;

	int	i, rs, len ;
	int	child_stat ;
	int	efd ;
	int	f_maxsleep = FALSE ;

	char	logid[JOBDB_JOBIDLEN + 1] ;
	char	timebuf[TIMEBUFLEN], timebuf2[TIMEBUFLEN] ;
	char	*cp ;


	jobdb_init(&jdb,1,gp->tmpdir) ;


/* enter this job into the database */

	bufprintf(logid,JOBDB_JOBIDLEN,"%d.%d",gp->ppid,0) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("watchone: logid=%s\n",logid) ;
#endif

	if ((rs = jobdb_newjob(&jdb,logid)) >= 0) {

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("watchone: newjob ji=%d\n",rs) ;
#endif

	    rs = jobdb_get(&jdb,rs,&jep) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("watchone: jobdb_get rs=%d\n",rs) ;
#endif

	    logfile_setid(&gp->lh,jep->jobid) ;


	    rs = SR_NXIO ;
	    pid = uc_fork() ;

	    if (pid == 0) {
		struct ustat	sb ;

/* we are now the CHILD!! */

	        gp->pid = getpid() ;

#ifdef	COMMENT
	        for (i = 0 ; i < 3 ; i += 1)
	            if (i != ns) u_close(i) ;

	        u_dup(ns) ;
#else
		if (! gp->f.fd_stdout) {

			u_close(FD_STDOUT) ;

	        	u_dup(ns) ;

		}
#endif /* COMMENT */

/* open the error file */

	        u_open(jep->efname,O_WRONLY,0666) ;

/* call the handler */

	        rs = handle(gp,pbp,ns,FD_STDOUT,elp,sfp,bip) ;

	        logfile_close(&gp->lh) ;

	        if (rs < 0)
	            u_unlink(jep->efname) ;

	        uc_exit((rs < 0) ? 1 : 0) ;

	    } /* end if */

/* parent is here */

#ifdef	COMMENT
	    while ((rs = u_waitpid(-1,&child_stat,W_OPTIONS)) != pid) {

	        if (! f_maxsleep) {

	            (void) time(&daytime) ;

	            if ((daytime - jep->atime) < 60)
	                sleep(daytime - jep->atime) ;

	            else
	                f_maxsleep = TRUE ;

	        } else
	            sleep(60) ;

	    } /* end while */
#else /* COMMENT */
	    rs = u_waitpid(pid,&child_stat,0) ;
#endif /* COMMENT */


/* process this guy's termination */

	    if ((efd = u_open(jep->efname,O_RDONLY,0666)) >= 0) {

	        rs = SR_OK ;
	        (void) time(&daytime) ;

	        logfile_printf(&g.lh, "%s server exit, es=%d\n",
	            timestr_log(daytime,timebuf),
	            child_stat & 255) ;

	        writeout(gp,efd,"* standard error *") ;

		logfile_printf(&gp->lh,"elapsed time %s\n",
			timestr_elapsed(daytime - jep->atime,timebuf)) ;

	    } else
	        logfile_printf(&gp->lh,"server did not 'exec'\n") ;

	    if (pid < 0) {

	        rs = pid ;
	        logfile_printf(&gp->lh,"we couldn't do a fork (rs=%d)\n",
	            pid) ;

	    }

	} else {

	    cp = "- could not allocate job resource" ;
	    uc_writen(ns,cp,strlen(cp)) ;

	    logfile_printf(&gp->lh,"jobdb new failed, rs=%d\n",rs) ;

	}

#ifdef	COMMENT
	jobdb_delp(&jdb,jep) ;
#endif /* COMMENT */

	jobdb_free(&jdb) ;

	return rs ;
}
/* end subroutine (watchone) */



/* LOCAL SUBROUTINES */



static void int_all(sn)
int	sn ;
{


	g.f_exit = TRUE ;
}
/* end subroutine (int_all) */


/* write out the output files from the executed program */
static int writeout(gp,fd,s)
struct global	*gp ;
int	fd ;
char	s[] ;
{
	bfile		file, *fp = &file ;

	struct ustat	sb ;

	int		tlen, len ;

	char		linebuf[LINELEN + 1] ;


	tlen = 0 ;
	if ((u_fstat(fd,&sb) >= 0) && (sb.st_size > 0)) {

	    u_rewind(fd) ;

	    logfile_printf(&gp->lh,s) ;

	    if (bopen(fp,(char *) fd,"dr",0666) >= 0) {

	        while ((len = breadline(fp,linebuf,LINELEN)) > 0) {

	            tlen += len ;
	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&gp->lh,"| %W\n",linebuf,MAX(len,62)) ;

	        } /* end while (reading lines) */

	        bclose(fp) ;

	    } /* end if (opening file) */

	} /* end if (non-zero file size) */

	return tlen ;
}
/* end subroutine (writeout) */



