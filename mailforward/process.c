/* process */

/* delivers mail messages (data) to a mailbox spool file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	F_TESTSLEEP	0		/* test sleep mode */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to deliver new mail to the mail spool file for a
        given recipient.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<sbuf.h>
#include	<bfile.h>
#include	<ctdec.h>
#include	<localmisc.h>

#include	"lkmail.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	BUFLEN		256

#define	TRIES_OUTER	20
#define	TRIES_INNER	4

#define	TO_RECORDLOCK	10

#define	FORWARDED	"Forward to "

#define	MAXMAILAGE	(5 * 60)

#define	MAILFILEMODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#ifndef	S_ISLNK
#define	S_ISLNK(mode)	((mode) & S_IFLNK)
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	getuid_name(cchar *,int) ;
extern int	isNotPresent(int) ;

extern int	copyparts(struct recip *,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct opens {
	uint	f_needsid : 1 ;		/* cache for need set-ID next time */
} ;


/* forward references */

static int	openspoolfile(struct proginfo *,struct opens *,const char *) ;
static int	createspoolfile(struct proginfo *,struct opens *,const char *,
			int) ;
static int	ourlock(int,int,int) ;
static int	mklockinfo(struct proginfo *,time_t,char *,int) ;


/* local variables */


/* exported subroutines */


int deliver(pip,tfd,rp)
struct proginfo	*pip ;
int		tfd ;
struct recip	*rp ;
{
	struct ustat	mstat ;

	struct opens	os ;

	LKMAIL		ml ;

	LKMAIL_IDS	ids ;

	sigset_t	oldsigmask, newsigmask ;

	time_t		daytime ;
	time_t		starttime ;

	offset_t	startoff ;

	uid_t		uid_recip ;

	int	rs, rs1 ;
	int	tlen ;
	int	i, j ;
	int	lfd, sfd ;
	int	buflen ;
	int	forlen ;
	int	mailage = MAXMAILAGE ;
	int	f_stat ;
	int	f_readable ;
	int	f_create ;

	char	mailfname[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("deliver: debuglevel=%u\n",pip->debuglevel) ;
	    debugprintf("deliver: euid=%d egid=%d\n",
	        geteuid(),getegid()) ;
	}
#endif

	if ((rp == NULL) || (rp->recipient[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("deliver: entered recip=%s\n",rp->recipient) ;
#endif

	memset(&os,0,sizeof(struct opens)) ;

/* verify that this recipient is valid and that we can write to her mailfile */

	f_stat = FALSE ;
	f_readable = FALSE ;
	f_create = FALSE ;

/* make the mailbox spool filename */

	rs = mkpath2(mailfname,pip->maildname,rp->recipient) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("deliver: mkpath2() rs=%d mailfname=%s\n",rs,mailfname) ;
#endif

	if (rs < 0)
	    return SR_INVALID ;

	if ((rs = u_lstat(mailfname,&mstat)) >= 0) {
	    uid_recip = mstat.st_uid ;
	    rs1 = u_stat(mailfname,&mstat) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("deliver: exists u_stat() rs=%d \n",rs) ;
#endif

	    if (rs1 >= 0) {
	        f_stat = TRUE ;
	        f_readable = S_ISREG(mstat.st_mode) &&
	            (mstat.st_mode & S_IRGRP) ;

	        if (S_ISDIR(mstat.st_mode)) rs = SR_ISDIR ;
	    }

	} else {

	    rs = SR_OK ;
	    if (! (pip->f.diverting && pip->f.trusted)) {
		if ((rs = getuid_name(rp->recipient,-1)) >= 0) {
	            uid_recip = rs ;
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
	            uid_recip = getuid() ;
		}
	    } else {
	        uid_recip = pip->uid_divert ;
	    }

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("deliver: determination rs=%d uid_recip=%d\n",
	        rs,uid_recip) ;
#endif

	if (rs < 0)
	    goto ret1 ;


/* set the information for the mail locking */

	ids.uid = pip->uid ;
	ids.euid = pip->euid ;
	ids.uid_maildir = pip->uid_maildir ;

	ids.gid = pip->gid ;
	ids.egid = pip->egid ;
	ids.gid_maildir = pip->gid_maildir ;

	rs = lkmail_start(&ml,mailfname,&ids) ;

	if (rs < 0)
	    goto ret1 ;


/* rewind the temporary message file */

	rs = u_seek(tfd,0L,SEEK_SET) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("deliver: tfd u_seek() rs=%d\n",rs) ;
	    u_fstat(tfd,&sb) ;
	    debugprintf("deliver: tfd size=%ld\n",sb.st_size) ;
	}
#endif

	forlen = strlen(FORWARDED) ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("deliver: before for\n") ;
#endif

	starttime = time(NULL) ;

	tlen = 0 ;
	sfd = SR_OK ;
	for (i = 0 ; i < TRIES_OUTER ; i += 1) {

/* lock the mail spool file */

	    daytime = time(NULL) ;

	    if ((daytime - starttime) > pip->timeout)
	        break ;

	    buflen = mklockinfo(pip,daytime,buf,BUFLEN) ;

	    if (buflen < 0)
	        buflen = ctdeci(buf,BUFLEN,pip->pid) ;


/* locking and file copying must be indivisible */

	    (void) sigemptyset(&newsigmask) ;

	    (void) sigaddset(&newsigmask,SIGALRM) ;

	    (void) sigaddset(&newsigmask,SIGPOLL) ;

	    (void) sigaddset(&newsigmask,SIGHUP) ;

	    (void) sigaddset(&newsigmask,SIGTERM) ;

	    (void) sigaddset(&newsigmask,SIGINT) ;

	    (void) sigaddset(&newsigmask,SIGQUIT) ;

	    (void) sigaddset(&newsigmask,SIGWINCH) ;

	    (void) sigaddset(&newsigmask,SIGURG) ;

#if	defined(PTHREAD) && PTHREAD
	    pthread_sigmask(SIG_BLOCK,&newsigmask,&oldsigmask) ;
#else
	    u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask) ;
#endif

/* try repeatedly to lock with the "lock" type file */

	    for (j = 0 ; j < TRIES_INNER ; j += 1) {

	        rs = lkmail_create(&ml) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("deliver: lkmail_create() rs=%d\n",rs) ;
#endif

	        if ((rs != SR_ACCES) && (rs != SR_TXTBSY))
	            break ;

/* we failed to capture the lock this time around */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("deliver: failed to capture lock file rs=%d\n",
	                rs) ;
#endif

/* check if the current mail lock is too old */

	        daytime = time(NULL) ;

	        rs1 = lkmail_old(&ml,daytime,mailage) ;

	        if (rs1 > 0) {

	            rs = lkmail_unlink(&ml) ;

	            if (rs >= 0) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("deliver: deleting old lockfile\n") ;
#endif

	                if (pip->f.log)
	                    logfile_printf(&pip->lh,
	                        "removed a stale mail lock\n") ;

/* try to get it again */

	                rs = lkmail_create(&ml) ;

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("deliver: lkmail_create() rs=%d\n",rs) ;
#endif

	                if ((rs != SR_ACCES) && (rs != SR_TXTBSY))
	                    break ;

	            } /* end if (we successfully removed the lock file) */

	        } /* end if (mail lock was too old) */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("deliver: about to sleep on lock\n") ;
#endif

	        if (j < (TRIES_INNER - 1))
	            sleep(1) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("deliver: woke up from sleep on lock\n") ;
#endif

	    } /* end for (locking with the "lock" type file) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("deliver: FILE lock create rs=%d\n",rs) ;
#endif

#ifdef	COMMENT
	    if (j >= TRIES_INNER)
	        lfd = SR_TXTBSY ;
#endif

/* if we have the mail lock, act on it */

	    lfd = rs ;
	    if (rs >= 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("deliver: we got the mail FILE lock\n") ;
#endif

/* put our PID and other information into the lockfile */

	        u_write(lfd,buf,buflen) ;

	        uc_fsync(lfd) ;

/* try to open the mail spool file */

#ifdef	COMMENT
	        if (pip->f.setgid) setegid(pip->egid) ;

	        if (pip->f.setgid) setegid(pip->gid) ;
#endif /* COMMENT */

	        rs = openspoolfile(pip,&os,mailfname) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("deliver: openspoolfile() rs=%d\n",rs) ;
#endif

	        sfd = rs ;
	        if (rs == SR_NOENT) {

	            rs = createspoolfile(pip,&os,mailfname,uid_recip) ;

	            sfd = rs ;
	            if (rs >= 0)
	                f_create = TRUE ;

	        } /* end if (could not open mailfile) */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("deliver: open spool file rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {

/* try to lock the mail spool file with UNIX System V Record Locking */

	            if ((! f_stat) ||
	                ((rs = ourlock(sfd,F_LOCK,TO_RECORDLOCK)) >= 0)) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("deliver: got the RECORD lock\n") ;
#endif

/* optionally check for mail forwarding */

	                if (pip->f.optforward && f_readable && 
				(! f_create) &&
	                    ((rs = u_read(sfd,buf,BUFLEN)) > 0)) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("deliver: read() rs=%d\n",
	                            rs) ;
#endif

	                    if (strncmp(buf,FORWARDED,forlen) == 0)
	                        rs = SR_REMOTE ;

	                } /* end if (read for forwarding test) */

	                if (rs >= 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("deliver: f_stat=%d u_seek()\n",
	                            f_stat) ;
#endif

	                    if (f_stat)
	                        rs = u_seek(sfd,0L,SEEK_END) ;

	                    if (rs >= 0) {

#if	F_TESTSLEEP
	                        sleep(20) ;
#endif

	                        u_tell(sfd,&startoff) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("deliver: startoff=%lu\n",
	                                startoff) ;
#endif

/* do the actual copy according to the specification on this recipient */

#ifdef	COMMENT

	                        rs = uc_copy(tfd,sfd,-1) ;

#if	CF_DEBUG
	                        if (pip->debuglevel > 1)
	                            debugprintf("deliver: uc_copy() rs=%d\n",
	                                rs) ;
#endif

#else /* COMMENT */

	                        rs = copyparts(rp,tfd,sfd) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("deliver: copyparts() rs=%d\n",
	                                rs) ;
#endif

#endif /* COMMENT */

	                        if (rs > 0)
	                            tlen = rs ;

	                        if (rs < 0)
	                            rs = uc_ftruncate(sfd,startoff) ;

	                    } /* end if (was able to seek) */

	                } /* end if (mail not being forwarded) */

/* unlock the mail spool file */

	                uc_fsync(sfd) ;

	                ourlock(sfd,F_ULOCK,10) ;

	            } /* end if (we got the record lock) */

	            u_close(sfd) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("deliver: after spoolfile close rs=%d\n",
	                    rs) ;
#endif

	        } /* end if (open or created the mail spool file) */

/* unlock mail spool area by removing the lock file */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("deliver: unlock spool file, rs=%d\n",rs) ;
#endif

	        u_close(lfd) ;

	        lkmail_unlink(&ml) ;

	    } /* end if (of handling new mail) */

/* turn interrupts, etc back on */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("deliver: interrupts on\n") ;
#endif

#if	defined(PTHREAD) && PTHREAD
	    pthread_sigmask(SIG_SETMASK,&oldsigmask,NULL) ;
#else
	    u_sigprocmask(SIG_BLOCK,&oldsigmask,NULL) ;
#endif

/* get out for all other bad reasons */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("deliver: rs=%d\n",rs) ;
#endif

	    if ((rs < 0) && (rs != SR_ACCESS) && (rs != SR_TXTBSY))
	        break ;

/* get out now if we were successful */

	    if (rs >= 0)
	        break ;

/* sleep (and try again) if we are not at the end of the loop */

	    if (i < (TRIES_OUTER - 1))
	        sleep(1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("deliver: bottom for\n") ;
#endif

	} /* end for (outer) */


/* let's finish up and get out ! */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	    debugprintf("deliver: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

ret2:
	lkmail_finish(&ml) ;

ret1:

ret0:
	return (rs >= 0) ? ((int) startoff) : rs ;
}
/* end subroutine (deliver) */



/* LOCAL SUBROUTINES */



/* this subroutine both locks and unlocks ! (depending on 'cmd') */
static int ourlock(fd,cmd,timeout)
int	fd, cmd, timeout ;
{
	int	rs, i ;


	if (timeout < 0)
	    timeout = 100 ;

/* return if the file in question is not seekable ! */

	rs = u_seek(fd,0L,SEEK_SET) ;

	if (rs < 0)
	    return rs ;

/* we're good, go ! */

	switch (cmd) {

	case F_LOCK:
	case F_TLOCK:
	    for (i = 0 ; i < timeout ; i += 1) {

	        rs = uc_lockf(fd,F_TLOCK,0L) ;

	        if (rs >= 0)
	            break ;

	        sleep(1) ;

	    } /* end for */

	    break ;

	case F_ULOCK:
	    rs = uc_lockf(fd,F_ULOCK,0L) ;

	    break ;

	default:
	    rs = SR_INVAL ;

	} /* end switch */

	return rs ;
}
/* end subroutine (ourlock) */


static int mklockinfo(pip,daytime,buf,buflen)
struct proginfo	*pip ;
time_t		daytime ;
char		buf[] ;
int		buflen ;
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;
	char		timebuf[TIMEBUFLEN + 1] ;

	if ((rs = sbuf_start(&b,buf,buflen)) >= 0) {

/* line 1 */

	sbuf_deci(&b,(int) pip->pid) ;

	sbuf_char(&b,'\n') ;

/* line 2 */

	sbuf_strw(&b,pip->lockaddr,-1) ;

	sbuf_char(&b,'\n') ;

/* line 3 */

	sbuf_strw(&b,timestr_logz(daytime,timebuf),-1) ;

	sbuf_char(&b,' ') ;

	sbuf_strw(&b,pip->progname,-1) ;

	sbuf_char(&b,'\n') ;

/* line 4 */

	sbuf_strw(&b,"logid=",-1) ;

	sbuf_strw(&b,pip->logid,-1) ;

	sbuf_char(&b,'\n') ;

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (mklockinfo) */


/* open the spool file (using SUID or SGID as necessary) */
static int openspoolfile(pip,osp,mailfname)
struct proginfo	*pip ;
struct opens	*osp ;
const char	mailfname[] ;
{
	int	rs, rs1 ;
	int	euid, egid ;
	int	f_suid = FALSE ;
	int	f_sgid = FALSE ;


	rs = SR_ACCESS ;
	if (! osp->f_needsid) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("deliver/openspoolfile: 1st open mailfname=%s\n",
	            mailfname) ;
	        debugprintf("deliver/openspoolfile: euid=%d egid=%d\n",
	            geteuid(),getegid()) ;
	    }
#endif

	    rs = uc_open(mailfname,O_RDWR,0666) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("deliver/openspoolfile: uc_open() rs=%d\n",rs) ;
#endif

	} /* end if (trying without using some sort of SID) */

	if (rs == SR_ACCESS) {

	    if (pip->f.setuid) {

	        euid = geteuid() ;

	        if (euid != pip->euid) {

	            rs1 = u_seteuid(pip->euid) ;

	            f_suid = (rs1 >= 0) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("deliver/openspoolfile: u_seteuid() rs=%d"
	                    " euid=%d\n",
	                    rs1,geteuid()) ;
#endif

	        }

	    } /* end if (we have SUID) */

	    if (pip->f.setgid) {

	        egid = getegid() ;

	        if (egid != pip->egid) {

	            rs1 = u_setegid(pip->egid) ;

	            f_sgid = (rs1 >= 0) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("deliver/openspoolfile: u_setegid() rs=%d"
	                    " egid=%d\n",
	                    rs1,getegid()) ;
#endif

	        }

	    } /* end if (we have SGID) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("deliver/openspoolfile: 2nd open \n") ;
	        debugprintf("deliver/openspoolfile: euid=%d egid=%d\n",
	            geteuid(),getegid()) ;
	    }
#endif

	    if (f_suid || f_sgid) {

	        rs = uc_open(mailfname,O_RDWR,0666) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("deliver/openspoolfile: uc_open() rs=%d\n",rs) ;
#endif

	    }

/* restore as appropriate */

	    if (f_suid)
	        u_seteuid(euid) ;

	    if (f_sgid)
	        u_setegid(egid) ;

	    if (rs >= 0)
	        osp->f_needsid = TRUE ;

	} /* end if (failed on first time) */

	return rs ;
}
/* end subroutine (openspoolfile) */


static int createspoolfile(pip,osp,mailfname,uid_recip)
struct proginfo	*pip ;
struct opens	*osp ;
const char	mailfname[] ;
int		uid_recip ;
{
	struct ustat	sb ;

	int	rs, rs1 ;
	int	sfd ;
	int	euid, egid ;
	int	f_suid = FALSE ;
	int	f_sgid = FALSE ;


	rs = SR_ACCESS ;
	if (! osp->f_needsid) {

	    rs = u_creat(mailfname,0666) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("deliver: u_creat() rs=%d\n",rs) ;
#endif

	}

	if (rs == SR_ACCESS) {

	    if (pip->f.setuid) {

	        euid = geteuid() ;

	        if (euid != pip->euid) {

	            rs1 = u_seteuid(pip->euid) ;

	            f_suid = (rs1 >= 0) ;

	        }

	    } /* end if (we have SUID) */

	    if (pip->f.setgid) {

	        egid = getegid() ;

	        if (egid != pip->egid) {

	            rs1 = u_setegid(pip->egid) ;

	            f_sgid = (rs1 >= 0) ;

	        }

	    } /* end if (we have SGID) */

	    if (f_suid || f_sgid) {

	        rs = u_creat(mailfname,0666) ;

	    }

/* restore as appropriate */

	    if (f_suid)
	        u_seteuid(euid) ;

	    if (f_sgid)
	        u_setegid(egid) ;

	    if (rs >= 0)
	        osp->f_needsid = TRUE ;

	} /* end if (failed on first time) */

	sfd = rs ;
	if (rs >= 0) {

	    int	mode ;


	    rs1 = u_fstat(sfd,&sb) ;

	    if ((rs1 >= 0) && 
	        ((sb.st_mode & MAILFILEMODE) != MAILFILEMODE)) {

	        mode = sb.st_mode | MAILFILEMODE ;
	        u_fchmod(sfd,mode) ;

	    }

	    u_fchown(sfd,uid_recip,pip->gid_mail) ;

	} /* end if (needed to create mailfile) */

	return (rs >= 0) ? sfd : rs ;
}
/* end subroutine (createspoolfile) */



