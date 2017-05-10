/* process */

/* process the service names given us */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable debug print-outs */
#define	CF_ALWAYSDEFAULT	0		/* always want the default access ? */
#define	CF_FASTQ		0		/* fast Q allocation of PROEGENTRYs */
#define	CF_SRVTABCHECK	1		/* check SRVTAB for changes */
#define	CF_ACCTABCHECK	1		/* check ACCTAB for changes */
#define	CF_SRVTABFREE	0		/* free up SRVTAB occassionally ? */
#define	CF_ACCTABFREE	0		/* free up ACCTAB occassionally ? */


/* revision history:

	= 91/09/01, David A­D­ Morano

	This subroutine was adopted from the DWD program.


*/


/*****************************************************************************

	This subroutine is responsible for processing the jobs that
	have been handed to us from the initialization code.


	Returns:

	OK	may not really matter in the current implementation !



*****************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/time.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<varsub.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<sockaddress.h>
#include	<field.h>
#include	<srvtab.h>
#include	<acctab.h>
#include	<cq.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"

#ifdef	DMALLOC
#include	<dmalloc.h>
#endif


/* local defines */

#define	W_OPTIONS	(WNOHANG)
#define	MAXOUTLEN	61
#define	O_SRVFLAGS	(O_RDWR | O_CREAT)

#define	TO_LOCKFILE	0		/* lock timeout */
#define	TO_READ		5		/* read timeout */

#define	TI_POLL		2		/* poll interval */
#define	TI_TIMEOUT	45		/* miscellaneous timeout */
#define	TI_MAINT	(3 * 60)	/* miscellaneous maintenance */

#define	POLLSEC		1000

#define	JOBFRACTION	10

#ifndef	XDEBFILE
#define	XDEBFILE		"/var/tmp/pcspoll.deb"
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(3 * MAXHOSTNAMELEN)
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snddd(char *,int,uint,uint) ;
extern int	cfdeci(char *,int,int *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getpwd(char *,int) ;
extern int	checklockfile(struct proginfo *,bfile *,char *,char *,
			time_t,pid_t) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* externals variables */


/* local structures */

struct pollentry {
	uint	lastsize ;
	char	mailuser[MAILUSERLEN + 1] ;
	char	mailfname[MAXPATHLEN + 1] ;
} ;

struct procinfo {
	vecobj	polls ;			/* things to poll */
	time_t	daytime ;
	time_t	t_lockcheck, t_pidcheck ;
	int	s ;
	int	serial ;		/* serial number */
	int	f_error ;
} ;


/* forward references */

static int	process_cycle(struct proginfo *,struct procinfo *) ;
static int	process_report(struct proginfo *,struct procinfo *,
			struct pollentry *) ;

static void	int_all(int) ;


/* local variables */

static int	f_exit ;







int process(pip,snp)
struct proginfo		*pip ;
vecstr			*snp ;		/* initial service names */
{
	struct procinfo		state, *sip ;

	struct sigaction	sigs ;

	struct ustat		sb ;

	struct pollfd	fds[2] ;

	sigset_t	signalmask ;

	time_t		t_start, t_check ;
	time_t		t_free ;

#ifdef	DMALLOC
	unsigned long	dmallocmark ;
#endif

	int	rs, rs1, rs2, i ;
	int	loopcount = 0 ;
	int	serial, sl, cl ;
	int	ti_poll ;
	int	ti_jobslice, ti_check, ti_minjob ;
#if	defined(DMALLOC) || defined(MALLOCLOG)
	int	dmalloc_count = 0 ;
#endif
	int	f ;

	char	timebuf[TIMEBUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	*sp, *cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: entered \n") ;
#endif

	memset(sip,0,sizeof(struct procinfo)) ;

	sip->daytime = time(NULL) ;

	t_start = sip->daytime ;
	t_check = sip->daytime ;
	serial = 0 ;

/* set up for the work period */

	rs = vecobj_start(&sip->polls,sizeof(struct pollentry),10,0) ;

	if (rs < 0)
		goto bad0 ;

	for (i = 0 ; vecstr_get(snp,i,&sp) >= 0 ; i += 1) {

		mkpath2(tmpfname,pip->maildname,sp) ;

		rs1 = u_stat(tmpfname,&sb) ;

		if ((rs1 >= 0) && (! S_ISDIR(sb.st_mode))) {

			struct pollentry	e ;


			memset(&e,0,sizeof(struct pollentry)) ;

			e.lastsize = 0 ;
			strwcpy(e.mailuser,sp,MAILUSERLEN) ;

			strcpy(e.mailfname,tmpfname) ;

			rs = vecobj_add(&sip->polls,&e) ;

		if (rs < 0)
			break ;

		}

	} /* end for */

	if (rs < 0)
		goto bad1 ;


/* open a socket for sending */

	{

		rs = listenudp(NULL,PORTSPEC,0) ;

		if (rs < 0)
			goto bad1 ;

		sip->s = rs ;

	} /* end block */


/* screw the stupid signals */

	memset(&sigs,0,sizeof(struct sigaction)) ;

	(void) uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGTERM,&sigs,NULL) ;

	(void) uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGHUP,&sigs,NULL) ;

	(void) uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGINT,&sigs,NULL) ;


	
/* top of loop */

	loopcount = 0 ;
	f_exit = FALSE ;
	while (! f_exit) {

	    rs = process_cycle(pip,sip) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: process_cycle() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        break ;

	    sip->daytime = time(NULL) ;

	        if (pip->runtime >= 0) {

	            if (sip->daytime > (t_start + pip->runtime))
	                break ;

	        }

		t_check += pip->interval ;
	        ti_poll = MAX((t_check - sip->daytime),0) ;

		if (ti_poll > TI_TIMEOUT)
			ti_poll = TI_TIMEOUT ;

	if (ti_poll > 0) {

	fds[0].fd = -1 ;
	u_poll(fds,0,(ti_poll * POLLSEC)) ;

	}

/* check up on the log file */

	    if (pip->f.log)
		logfile_check(&pip->lh,sip->daytime) ;

	    loopcount += 1 ;

	} /* end while (looping) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: ret f_exit=%d\n",f_exit) ;
#endif


#if	CF_LOGONLY
	f = pip->f.daemon ;
#else
	f = TRUE ;
#endif

	if (f) {

	    sip->daytime = time(NULL) ;

	    if (pip->f.daemon && f_exit)
	        cp = "%s server exiting (interrupt)\n" ;

	    else if (pip->f.daemon)
	        cp = "%s server exiting (period expired)\n" ;

	    else
	        cp = "%s exiting (work completed)\n" ;

	    if (pip->f.log)
	        logfile_printf(&pip->lh,cp,
	            timestr_logz(sip->daytime,timebuf)) ;

	} /* end if (daemon mode exit stuff) */

	rs = SR_OK ;


/* early and regular exits */
badlockfile:
badpidfile:

badhup:
baderr:

bad4:

bad2:
	u_close(sip->s) ;

bad1:
	vecobj_finish(&sip->polls) ;

bad0:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */



/* LOCAL SUBROUTINES */



/* do a cycle of work */
static int process_cycle(pip,sip)
struct proginfo	*pip ;
struct procinfo	*sip ;
{
	struct pollentry	*pep ;

	struct ustat		sb ;

	int	rs = SR_OK, rs1, i ;

	char	timebuf[TIMEBUFLEN + 1] ;


/* do the polls */

	for (i = 0 ; vecobj_get(&sip->polls,i,&pep) >= 0 ; i += 1) {

		if (pep == NULL) continue ;

		rs1 = u_stat(pep->mailfname,&sb) ;

		if (rs1 >= 0) {

			if (sb.st_size > pep->lastsize) {

				rs = process_report(pip,sip,pep) ;

			} /* end if (new size was greater) */

			pep->lastsize = sb.st_size ;

		} else
			pep->lastsize = 0 ;

		if (rs < 0)
			break ;

	} /* end for */

	if (rs < 0)
		goto ret0 ;

/* do other stuff for this cycle */

	        if (pip->f.log)
	            logfile_setid(&pip->lh,pip->logid) ;

/* maintenance the lock file */

	if (! pip->f.named) {

	    if ((pip->lockfp != NULL) && 
	        ((sip->daytime - sip->t_lockcheck) > TI_MAINT)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process_cycle: checking LOCK file\n") ;
#endif

	        rs = checklockfile(pip,pip->lockfp,pip->lockfname,
	            BANNER,sip->daytime,pip->pid) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process_cycle: checklockfile() rs=%d\n",rs) ;
#endif

	        if (rs != 0) {

	            if (rs > 0) {

	                if (pip->f.log)
	                    logfile_printf(&pip->lh,
	                        "%s another program has my lock file, "
	                        "other PID=%d\n",
	                        timestr_logz(sip->daytime,timebuf),
	                        rs) ;

	            } else {

	                if (pip->f.log)
	                    logfile_printf(&pip->lh,
	                        "%s tampered lock file, rs=%d\n",
	                        timestr_logz(sip->daytime,timebuf),
	                        rs) ;

	            }

	            rs = SR_ALREADY ;
		    sip->f_error = TRUE ;
	            goto badlockfile ;
	        }

	        sip->t_lockcheck = sip->daytime ;

	    } /* end if (maintaining the lock file) */

	} /* end if */

/* maintenance the PID mutex lock file */

	if (pip->f.daemon && (pip->pidfp != NULL) && 
	    ((sip->daytime - sip->t_pidcheck) > TI_MAINT)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process_cycle: PID file pid=%d\n",
			pip->pid) ;
#endif

	    rs = checklockfile(pip,pip->pidfp,pip->pidfname,
	        BANNER,sip->daytime,pip->pid) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process_cycle: checklockfile() rs=%d\n",rs) ;
#endif

	    if (rs != 0) {

	        if (rs > 0) {

	            if (pip->f.log)
	                logfile_printf(&pip->lh,
	                    "%s another program has PID file, other PID=%d\n",
	                    timestr_logz(sip->daytime,timebuf),
	                    rs) ;

	        } else {

	            if (pip->f.log)
	                logfile_printf(&pip->lh,
	                    "%s something happened to PID file, rs=%d\n",
	                    timestr_logz(sip->daytime,timebuf),
	                    rs) ;

	        }

	        rs = SR_ALREADY ;
		sip->f_error = TRUE ;
	        goto badpidfile ;
	    }

	    sip->t_pidcheck = sip->daytime ;

	} /* end if (maintaining the PID mutex file) */

ret0:
	return rs ;

/* bad things */
badpidfile:
badlockfile:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process_cycle: ret BAD rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process_cycle) */


static int process_report(pip,sip,pep)
struct proginfo		*pip ;
struct procinfo		*sip ;
struct pollentry	*pep ;
{
	PARAMFILE_CUR	cur ;

	SOCKADDRESS		to ;

	int	rs, bl, vl ;

	char	buf[MSGBUFLEN + 1] ;
	char	vbuf[VBUFLEN + 1] ;
	char	*mailuser = pep->mailuser ;


/* create the message */

	buf[0] = '\0' ;
	bl = 0 ;
	if (mailuser != NULL)
		bl = strwcpy(buf,mailuser,LOGNAMELEN) - buf ;

/* find the mailuser's target machine */

	paramfile_curbegin(&pip->mbtab,&cur) ;

	while (TRUE) {

		vl = paramfile_fetch(&pip->mbtab,mailuser,&cur,vbuf,VBUFLEN) ;

		if (vl < 0)
			break ;

/* create address to send to */

		sockaddress_start() ;


/* send the message */

	rs = u_sendto(sip->s,buf,bl,0,&to) ;

/* is there a reply ? */

	if (rs >= 0) {

		rs = uc_recve(sip->s,buf,MSGBUFLEN,0,TO_READ,0) ;

		if ((rs == 0) || ((rs > 0) && (buf[0] != '\0')))
			rs = SR_INVALID ;

	}

	} /* end while */

	paramfile_curend(&pip->mbtab,&cur) ;

ret0:
	return rs ;
}
/* end subroutine (process_cycle) */


static void int_all(sn)
int	sn ;
{


	f_exit = TRUE ;
}
/* end subroutine (int_all) */



