/* watch */

/* watch (listen on) the specified socket */
/* version %I% last modified %G% */


#define	CF_DEBUG	1		/* switchable debug print-outs */
#define	F_REQUESTS	1		/* handle FIFO requests, usually on */


/* revision history:

	= 91/09/01, David A­D­ Morano

	This subroutine was adopted from the DWD program.


*/



/*****************************************************************************

	This subroutine is responsible for listening on the given
	socket and spawning off a program to handle any incoming
	connection.

	This subroutine is called with the folowing arguments :
	pip	program information pointer
	s	socket to listen on
	ssp	varsub substitutions to be made on server table stuff
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
#include	<sys/msg.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<time.h>
#include	<ftw.h>
#include	<dirent.h>
#include	<limits.h>
#include	<string.h>
#include	<stropts.h>
#include	<poll.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<baops.h>
#include	<exitcodes.h>

#include	"srvtab.h"
#include	"acctab.h"

#include	"localmisc.h"
#include	"jobdb.h"
#include	"builtin.h"
#include	"config.h"
#include	"defs.h"
#include	"standing.h"



/* local defines */

#define	W_OPTIONS	(WNOHANG)
#define	TI_POLL		2		/* u_poll(2) interval */
#define	TI_MAINT	(3 * 60)	/* miscellaneous maintenance */
#define	MAXOUTLEN	62
#define	O_SRVFLAGS	(O_RDWR | O_CREAT )


/* external subroutines */

extern int	opentmpfile(char *,int,mode_t,char *) ;
extern int	checklockfile(struct proginfo *,bfile *,char *,char *,
			time_t,pid_t) ;
extern int	handle(struct proginfo *, struct serverinfo *, 
			BUILTIN *,STANDING *, struct clientinfo *) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;

#if	CF_DEBUG
extern char	*d_reventstr() ;
#endif


/* externals variables */


/* forward references */

static int	writeout() ;
static int	write_mqid() ;
static int	watch_newjob(struct proginfo *, struct serverinfo *,
			BUILTIN *, STANDING *, struct clientinfo *,int,int) ;

static void	int_all(int) ;


/* local global variables */

static int	f_exit ;


/* local structures */


/* local data */






int watch(pip,sip,ssp,elp,sfp,atp,bip)
struct proginfo		*pip ;
struct serverinfo	*sip ;
varsub		*ssp ;
vecstr		*elp ;			/* exports */
SRVTAB		*sfp ;
ACCTAB		*atp ;
BUILTIN		*bip ;
{
	struct pollfd	fds[3] ;

	STANDING	ourstand ;

	SERIALBUF	reqbuf ;

	struct msgbuffer	mb ;

	struct sigaction	sigs ;

	struct ustat		sb ;

	struct clientinfo	ci, *cip = &ci ;

	JOBDB_ENT	*jep ;

	pid_t		pid ;

	sigset_t	signalmask ;

	time_t		daytime ;
	time_t		t_lockcheck = 1 ;
	time_t		t_pidcheck = 1 ;
	time_t		t_lastmark = 0 ;

	int	i, len ;
	int	rs = SR_BAD ;
	int	to_pollidle = TI_POLL * 1000 ;
	int	to_poll ;
	int	loopcount = 0 ;
	int	njobs = 0 ;
	int	child_stat ;
	int	nfds, nfd ;
	int	re ;
	int	fromlen ;
	int	ns ;
	int	efd ;
	int	f_tmpfifo = FALSE ;

	char	logid[JOBDB_JOBIDLEN + 1] ;
	char	reqfname[MAXPATHLEN + 2] ;
	char	tmpfname[MAXPATHLEN + 2], *ipcbuf = tmpfname ;
	char	timebuf[TIMEBUFLEN] ;
	char	*cp ;

#if	CF_DEBUG
	char	tmpbuf[BUFLEN + 1] ;
#endif


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: entered \n") ;
#endif


	sip->serial = 0 ;

	f_exit = FALSE ;
	rs = jobdb_init(&sip->jobs,4,pip->tmpdname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: jobdb_init rs=%d\n",rs) ;
#endif


/* make the FIFO for IPC requests */

	if (pip->f.daemon) {

	            (void) u_time(&daytime) ;

		t_lastmark = daytime ;

		strcpy(reqfname,pip->spooldname) ;

	    strcat(reqfname,"/") ;

	    strcat(reqfname,REQFNAME) ;

	    rs = u_stat(reqfname,&sb) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("watch: stat rs=%d mode=%08o\n",rs,sb.st_mode) ;
#endif

	    if (rs < 0) {

	        (void) u_unlink(reqfname) ;

	        (void) uc_mkfifo(reqfname,0662) ;

	    } else if (! S_ISFIFO(sb.st_mode)) {

	        (void) u_unlink(reqfname) ;

	        (void) uc_mkfifo(reqfname,0662) ;

	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("watch: FIFO filename=%s\n",reqfname) ;
#endif

	    rs = u_open(reqfname,O_SRVFLAGS,0666) ;

		if (rs < 0) {

	        (void) u_unlink(reqfname) ;

	        (void) uc_mkfifo(reqfname,0662) ;

	    	rs = u_open(reqfname,O_SRVFLAGS,0666) ;

		}

	} else {

		f_tmpfifo = TRUE ;
	    rs = opentmpfile("/tmp/tmXXXXXXXXXXXX",
	        O_SRVFLAGS,(S_IFIFO | 0622),reqfname) ;

	} /* end if (request FIFO creation) */

	sip->fd_ipc = rs ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    u_fstat(sip->fd_ipc,&sb) ;

	    debugprintf("watch: open FIFO rs=%d mode=%8o\n",
	        rs,sb.st_mode) ;

	}
#endif

	if (rs < 0)
	    goto bad1 ;


/* set message discard mode on the FIFO (message oriented) */

	rs = u_ioctl(sip->fd_ipc,I_SRDOPT,RMSGD) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: message discard mode, rs=%d\n",rs) ;
#endif


/* create the message queue for sending IPC results back */

	rs = u_msgget(IPC_PRIVATE,0600 | IPC_CREAT) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: u_msgget rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;

	sip->mqid_ipc = rs ;
	if (pip->f.daemon) {

	    rs = write_mqid(pip,sip->mqid_ipc) ;

	    if (rs < 0) {

	        logfile_printf(&pip->lh,
	            "could not write MSGQ ID file (rs %d)\n",rs) ;

	        bprintf(pip->efp,
	            "%s: could not write MSQQ ID file (rs %d)\n",
	            pip->progname,rs) ;

	        goto bad3 ;
	    }

	} /* end if (daemon mode) */


/* initialize the standing server part */

	standing_init(&ourstand,pip,sip) ;


/* we want to receive the new socket (from 'accept') above these guys */

	if (pip->f.daemon) {

	    for (i = 0 ; i < 3 ; i += 1) {

	        if (u_fstat(i,&sb) < 0)
	            (void) u_open("/dev/null",O_RDONLY,0666) ;

	    }


	} /* end if (daemon mode) */


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

	if (pip->f.daemon) {

	    fds[nfds].fd = sip->fd_listentcp ;
	    fds[nfds].events = POLLIN | POLLPRI ;
	    fds[nfds].revents = 0 ;
	    nfds += 1 ;

	}

	fds[nfds].fd = sip->fd_ipc ;
	fds[nfds].events = POLLIN | POLLPRI ;
	fds[nfds].revents = 0 ;
	nfds += 1 ;

	fds[nfds].fd = -1 ;


/* if we are not in daemon mode, then we have a job waiting on FD_STDIN */

	if (! pip->f.daemon) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        d_whoopen("not daemon initial") ;
#endif

	    rs = watch_newjob(pip,sip,bip,&ourstand,cip,FD_STDIN,FD_STDOUT) ;

	    if (rs < 0) {

	        cp = "- could not allocate job resource" ;
	        uc_writen(FD_STDOUT,cp,strlen(cp)) ;

	        logfile_printf(&pip->lh,"jobdb new failed, rs=%d\n",rs) ;

	    }

	} /* end if (not running daemon mode) */


	loopcount = 0 ;

/* top of loop */
top:
	cip->salen = -1 ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: about to poll\n") ;
#endif

	to_poll = (njobs <= 0) ? to_pollidle : (to_pollidle / 2) ;
	rs = u_poll(fds,nfds,to_poll) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: back from poll w/ rs=%d\n",rs) ;
#endif

	if (rs > 0) {

	    for (nfd = 0 ; nfd < nfds ; nfd += 1) {

/* handle any activity on our listen socket */

	        if ((fds[nfd].fd == sip->fd_listentcp) &&
	            ((re = fds[nfd].revents) != 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("watch: back from poll, FD=%d re=%s\n",
	                    fds[nfd].fd,
	                    d_reventstr(re,tmpbuf,BUFLEN)) ;
#endif

	            if ((re & POLLIN) || (re & POLLPRI)) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("watch: got a poll in, n=%d p=%d\n",
	                        (re & POLLIN) ? 1 : 0,(re & POLLPRI) ? 1 : 0) ;
#endif

	                fromlen = sizeof(struct sockaddr) ;
	                ns = u_accept(sip->fd_listentcp,&cip->sa,&fromlen) ;

	                cip->salen = fromlen ;

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("watch: we accepted a call, rs=%d\n",ns) ;
#endif

	                if (ns < 0) {

#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        debugprintf("watch: u_accept rs=%d\n",
	                            ns) ;
#endif

	                    goto baderr ;
	                }


#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    d_whoopen("from accept") ;
#endif

	                rs = watch_newjob(pip,sip,bip,&ourstand,cip,ns,ns) ;


	                u_close(ns) ;


	                if (rs < 0) {

	                    cp = "- could not allocate job resource" ;
	                    uc_writen(ns,cp,strlen(cp)) ;

	                    logfile_printf(&pip->lh,
				"jobdb new failed, rs=%d\n",rs) ;

	                } else
	                    sip->serial += 1 ;

	                logfile_setid(&pip->lh,pip->logid) ;

	            } else if (re & POLLHUP)
	                goto badhup ;

	            else if (re & POLLERR)
	                goto baderr ;

	        } /* end if (our listen socket) */


/* handle any activity on our request FIFO */

#if	F_REQUESTS
	        if ((fds[nfd].fd == sip->fd_ipc) &&
	            ((re = fds[nfd].revents) != 0)) {

	            if ((re & POLLIN) || (re & POLLPRI)) {

	                uint	rcode ;

	                uchar	ch ;


#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("watch: back from poll, FD=%d re=%s\n",
	                        fds[nfd].fd,
	                        d_reventstr(re,tmpbuf,BUFLEN)) ;
#endif

	                len = u_read(sip->fd_ipc,ipcbuf,MAXPATHLEN) ;

#if	CF_DEBUG
	                if (pip->debuglevel > 1) {
	                    debugprintf("watch: IPC FIFO read, rs=%d\n",len) ;
	                    debugprintf("watch: ") ;
	                    for (i = 0 ; i < 10 ; i += 1)
	                        debugprintf(" %02x",ipcbuf[i]) ;
	                    debugprintf("\n") ;
	                }
#endif /* CF_DEBUG */

	                if (len > 0) {

	                    serialbuf_start(&reqbuf,ipcbuf,len) ;

	                    serialbuf_remchar(&reqbuf,&ch) ;

	                    rcode = (uint) ch ;

#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        debugprintf("watch: IPC rcode=%d\n",rcode) ;
#endif

/* what do we do with this code ? */

	                    switch (rcode) {

	                    case RCODE_NOOP:
	                    case RCODE_EXIT:
	                    case RCODE_PASSFD:

#if	CF_DEBUG
	                        if (pip->debuglevel > 1)
	                            debugprintf("watch: IPC not implemented\n") ;
#endif

	                        break ;

	                    default:

#if	CF_DEBUG
	                        if (pip->debuglevel > 1)
	                            debugprintf("watch: IPC standing\n") ;
#endif

	                        rs = standing_request(&ourstand,rcode,&reqbuf) ;

	                    } /* end switch */

	                    serialbuf_finish(&reqbuf) ;

	                } /* end if (non-zero length) */

	            } /* end if (readable) */


	        } /* end if (our request FIFO) */
#endif /* F_REQUESTS */

	    } /* end for */

	} /* end if (something from 'poll') */


/* are there any completed jobs yet ? */

	if ((rs = u_waitpid(-1,&child_stat,W_OPTIONS)) > 0) {

	    int	ji ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("watch: child exit, pid=%d stat=%d\n",
	            rs,(child_stat & 0xFF)) ;
#endif

	    pid = rs ;
	    if ((ji = jobdb_findpid(&sip->jobs,pid,&jep)) >= 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("watch: found child, ji=%d\n",ji) ;
#endif

	        logfile_setid(&pip->lh,jep->jobid) ;

/* process this guy's termination */

	        if ((efd = u_open(jep->efname,O_RDONLY,0666)) >= 0) {

	            rs = SR_OK ;
	            (void) u_time(&daytime) ;

	            logfile_printf(&pip->lh, "%s server exit, ex=%d\n",
	                timestr_log(daytime,timebuf),
	                child_stat & 255) ;

	            writeout(pip,efd,"standard error") ;

	            logfile_printf(&pip->lh,"elapsed time %s\n",
	                timestr_elapsed((daytime - jep->atime),timebuf)) ;

	            u_close(efd) ;

	        } else {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("watch: child did not 'exec'\n") ;
#endif

	            logfile_printf(&pip->lh,"server did not 'exec'\n") ;

	        }

	        jobdb_del(&sip->jobs,ji) ;

	        logfile_setid(&pip->lh,pip->logid) ;

/* flush any messages that the process did not read */

	        while (TRUE) {

	            long	type = (int) (ji + 1) ;


	            rs = u_msgrcv(sip->mqid_ipc,&mb,MSGBUFLEN,
	                type,IPC_NOWAIT) ;

	            if (rs < 0)
	                break ;

	        } /* end while */


	        if (! pip->f.daemon)
	            f_exit = TRUE ;

	    } else
	        logfile_printf(&pip->lh,"unknown PID=%d\n",rs) ;

	} /* end if (a child process exited) */


/* maintenance the lock file */

	if (pip->f.daemon) {

	    (void) time(&daytime) ;

	    if ((pip->lockfp != NULL) && 
	        ((daytime - t_lockcheck) >= (TI_MAINT - 1))) {

	        rs = checklockfile(pip,pip->lockfp,pip->lockfname,
			BANNER,daytime,pip->pid) ;

	        if (rs != 0) {

	            logfile_printf(&pip->lh,
	                "%s another program has my lock file, other PID=%d\n",
	                timestr_log(daytime,timebuf),
	                rs) ;

	            goto badlockfile ;
	        }

	        t_lockcheck = daytime ;

	    } /* end if (maintaining the lock file) */

	} /* end if */


/* maintenance the PID mutex lock file */

	if (pip->f.daemon && (pip->pidfp != NULL) && 
	    ((daytime - t_pidcheck) >= (TI_MAINT - 1))) {

	    rs = checklockfile(pip,pip->pidfp,pip->pidfname,
			BANNER,daytime,pip->pid) ;

	    if (rs != 0) {

	        logfile_printf(&pip->lh,
	            "%s another program has my PID file, other PID=%d\n",
	            timestr_log(daytime,timebuf),
	            rs) ;

	        goto badpidfile ;
	    }

	    t_pidcheck = daytime ;

	} /* end if (maintaining the PID mutex file) */


/* check up on the standing server object */

	standing_check(&ourstand) ;


/* check if the server table file (srvtab) has changed */

	if (pip->f.daemon) {

	    if ((rs = srvtab_check(sfp,daytime,NULL)) > 0)
	        logfile_printf(&pip->lh,"%s server table file changed\n",
	            timestr_log(daytime,timebuf)) ;

		if (pip->marktime > 0) {

			if (pip->marktime < (daytime - t_lastmark)) {

				logfile_printf(&pip->lh,"%s mark> %s\n",
					timestr_log(daytime,timebuf),
					pip->nodename) ;

				t_lastmark = daytime ;
			}

		} /* end if */

	} /* end if (daemon mode) */


/* check if the access table has changed, if we have one */

	if (pip->f.daemon && pip->f.acctab) {

	    if ((rs = acctab_check(atp,NULL)) > 0)
	        logfile_printf(&pip->lh,"%s access table file changed\n",
	            timestr_log(daytime,timebuf)) ;

	}


	njobs = jobdb_count(&sip->jobs) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: jobs=%d\n",njobs) ;
#endif


/* go back to the top of the loop */

	loopcount += 1 ;
	if (! f_exit) goto top ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: returning !\n") ;
#endif

	if (f_tmpfifo)
		u_unlink(reqfname) ;

	if (pip->f.daemon) {

	    (void) time(&daytime) ;

	    logfile_printf(&pip->lh,"%s daemon exiting\n",
	        timestr_log(daytime,timebuf)) ;

	}

	rs = SR_OK ;


/* early and regular exits */
badlockfile:
badpidfile:

badhup:
baderr:

bad4:
	(void) standing_free(&ourstand) ;

bad3:
	u_msgctl(sip->mqid_ipc,IPC_RMID,NULL) ;		/* remove MSGQ */

bad2:
	u_close(sip->fd_ipc) ;

bad1:
	jobdb_free(&sip->jobs) ;

bad0:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (watch) */



/* LOCAL SUBROUTINES */



static void int_all(sn)
int	sn ;
{


	f_exit = TRUE ;
}
/* end subroutine (int_all) */


/* write out the output files from the executed program */
static int writeout(pip,fd,s)
struct proginfo	*pip ;
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

	    logfile_printf(&pip->lh,s) ;

	    if (bopen(fp,(char *) fd,"dr",0666) >= 0) {

	        while ((len = breadline(fp,linebuf,MAXOUTLEN)) > 0) {

	            tlen += len ;
	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&pip->lh,"| %W\n",
	                linebuf,MIN(len,MAXOUTLEN)) ;

	        } /* end while (reading lines) */

	        bclose(fp) ;

	    } /* end if (opening file) */

	} /* end if (non-zero file size) */

	return tlen ;
}
/* end subroutine (writeout) */


/* write our MSGQ ID to the MSGQ ID file */
static int write_mqid(pip,mqid)
struct proginfo	*pip ;
int		mqid ;
{
	bfile	mqfile ;

	int	rs ;

	char	tmpfname[MAXPATHLEN + 2] ;


	bufprintf(tmpfname,MAXPATHLEN + 1,"%s/%s",
	    pip->spooldname,MSGQFNAME) ;

	if ((rs = bopen(&mqfile,tmpfname,"wct",0664)) < 0)
	    return rs ;

	bprintf(&mqfile,"%d\n",mqid) ;

	bclose(&mqfile) ;

	return rs ;
}
/* end subroutine (write_mqid) */


/* spawn a job */
static int watch_newjob(pip,sip,bip,ourp,cip,nsi,nso)
struct proginfo		*pip ;
struct serverinfo	*sip ;
BUILTIN			*bip ;
STANDING		*ourp ;
struct clientinfo	*cip ;
int	nsi, nso ;
{
	JOBDB_ENT	*jep ;

	pid_t	pid ;

	int	rs, ji, i ;

	char	logid[JOBDB_JOBIDLEN + 1] ;


/* enter this job into the database */

	bufprintf(logid,JOBDB_JOBIDLEN,"%d.%d",
	    pip->serial,sip->serial) ;

	if ((rs = jobdb_newjob(&sip->jobs,logid,0)) < 0)
	    goto bad0 ;

	logfile_setid(&pip->lh,logid) ;

	ji = rs ;
	cip->mtype = (long) (ji + 2) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch_newjob: JID=%d mtype=%ld\n",
	        ji,cip->mtype) ;
#endif

	jobdb_get(&sip->jobs,ji,&jep) ;


/* let's fork the processing subroutine and get on with our lives ! */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch_newjob: about to fork, passing ns=%d\n",nsi) ;
#endif

	rs = uc_fork() ;
	pid = rs ;

	if (rs < 0) {
	    logfile_printf(&pip->lh,
	        "cannot fork (%d)\n",rs) ;
	    goto bad1 ;
	}

	if (pid == 0) {
	    int	f_nsiok, f_nsook ;

/* we are now the CHILD !! */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        d_whoopen("child 0") ;
#endif

	    pip->logid = jep->jobid ;
	    pip->pid = getpid() ;

/* close stuff we don't need */

	    if (pip->f.daemon)
	        u_close(sip->fd_listentcp) ;


/* move some FDs that we do need, if necessary */

	    f_nsiok = (nsi == FD_STDIN) ;

	    f_nsook = (nso == FD_STDOUT) ;

	    if ((nsi < 3) && (! f_nsiok))
	        nsi = uc_moveup(nsi,3) ;

	    if ((nso < 3) && (! f_nsook))
	        nso = uc_moveup(nso,3) ;

#if	CF_DEBUGS
	debugprintf("watch_newjob: 1 nsi=%d\n",nsi) ;
#endif

/* setup the input and output for the program */

	    for (i = 0 ; i < 3 ; i += 1) {

	        int	f_keep ;


	        f_keep = ((i == nsi) && f_nsiok) ;
	        if (! f_keep)
	            f_keep = ((i == nso) && f_nsook) ;

	        if (! f_keep)
	            (void) u_close(i) ;

	    } /* end for */

	    if (! f_nsiok) {

	        u_dup(nsi) ;

#if	CF_DEBUGS
		debugprintf("watch_newjob: duped to 0 ? nsi=%d\n",nsi) ;
		d_whoopen("after first dup") ;
#endif

	}

	    if (! f_nsook)
	        u_dup(nso) ;

	    u_open(jep->efname,O_WRONLY,0666) ;

/* close extras that we do not need (there are some that we keep) */

	    if (! f_nsiok)
	        u_close(nsi) ;

	    if (! f_nsook)
	        u_close(nso) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("watch_newjob: calling handle nsi=%d nso=%d\n",
			nsi,nso) ;
		d_whoopen("about to call handle()") ;
	}
#endif

/* do it */

	    rs = handle(pip,sip,bip,ourp,cip) ;

	    logfile_close(&pip->lh) ;

	    if (rs < 0)
	        u_unlink(jep->efname) ;

	    uc_exit(EC_NOEXEC) ;

	} /* end if */

	jep->pid = pid ;


	return rs ;

bad1:

#ifdef	COMMENT
	logfile_setid(&pip->lh,NULL) ;
#endif

	jobdb_del(ji) ;

bad0:

	return rs ;
}
/* end subroutine (watch_newjob) */



