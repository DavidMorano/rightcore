/* progspool */

/* get the mail messages out of the spool area */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_ACCESS	1		/* use 'u_access(3u)' */


/* revision history:

	= 1994-01-23, David A­D­ Morano
        This module was copied and modified from the original in VMAIL.
        Hopefully, this is a much more useful and portable function for cleanly
        retrieving the mail from the spool area.

	= 2000-08-02, David A­D­ Morano
	I finally got around to making this code squeaky POSIX-THREAD clean.  I
	changed the call of 'setprocmask' to 'pthread_sigmask'.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine copies the mail data out of the mail-spool file and
	into the file descriptor passed to us by our caller.

	Synopsis:

	int progspool(pip,mfd,flp,maildname,mailuser)
	struct proginfo	*pip ;
	int		mfd ;
	vecstr		*flp ;
	const char	maildname[] ;
	const char	mailuser[] ;

	Arguments:

	pip		program information pointer
	mfd		file descriptor to a file to receive any spool
			mail that is found
	flp		vecstr string list of forwarding mail addresses

	Returns:

	>0		there was new mail with length returned
	0		there was no new mail to get
	<0		error of some kind occurred


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	LIBUFLEN	256

#define	TRIES_OUTER	20
#define	TRIES_INNER	4

#define	TO_RECLOCK	10

#define	MAILSPOOL_FORWARDED	"Forward to"
#define	MAILSPOOL_FORWARDEDLEN	10


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct modinfo {
	VECSTR		*flp ;
	char		*mailfname ;
	char		*mlfname ;
	struct ustat	mdsb ;
	time_t		daytime ;
	int		lilen ;
	int		mfd ;
	char		libuf[LIBUFLEN + 1] ;
} ;


/* forward references */

static int	getspool_inner(struct proginfo *,struct modinfo *) ;
static int	mailcopy(struct proginfo *,const char *,int,vecstr *,int) ;

static int	mailcopy_forwarded(vecstr *,const char *,int) ;

#ifdef	COMMENT
static int	ourlock() ;
#endif /* COMMENT */

static int	maillock_create(struct proginfo *,const char *,struct ustat *) ;
static int	maillock_unlink(struct proginfo *,const char *,struct ustat *) ;

static int	mklockinfo(struct proginfo *,char *,int,time_t) ;


/* local variables */

static const int	sigblocks[] = {
	SIGPOLL,
	SIGHUP,
	SIGTERM,
	SIGINT,
	SIGQUIT,
	SIGWINCH,
	SIGURG,
	0
} ;


/* exported subroutines */


int progspool(pip,mfd,flp,maildname,mailuser)
struct proginfo	*pip ;
int		mfd ;
vecstr		*flp ;
const char	maildname[] ;
const char	mailuser[] ;
{
	struct modinfo	mi, *mip = &mi ;
	struct ustat	sf ;
	time_t		starttime ;
	offset_t	offset ;
	const int	pm = (R_OK|W_OK) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		tlen = 0 ;
	char		mailfname[MAXPATHLEN + 1] ;
	char		mlfname[MAXPATHLEN + 1] ;

	if (maildname[0] == '\0') return SR_INVALID ;
	if (mailuser[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progspool: maildname=%s mailuser=%s\n",
	        maildname,mailuser) ;
#endif

/* fill in the mail-spool information */

	memset(mip,0,sizeof(struct modinfo)) ;
	mip->mfd = mfd ;
	mip->flp = flp ;
	mip->mailfname = mailfname ;
	mip->mlfname = mlfname ;

/* check some easy things first */

	rs1 = u_stat(maildname,&mip->mdsb) ;
	if (rs1 < 0) goto ret0 ;

/* can we access the mail spool directory? */

#ifdef	COMMENT
	if (pip->f.setgid)
	    rs = perm(maildname,-1,pip->egid,NULL,W_OK) ;

	if (rs < 0)
	    rs = perm(maildname,-1,-1,NULL,W_OK) ;

	if ((rs < 0) && (pip->euid != 0))
	    goto ret0 ;
#endif /* COMMENT */

/* is there any new mail? */

	rs = mkpath2(mailfname,maildname,mailuser) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progspool: mailfname=%s\n",mailfname) ;
#endif

	if (rs > 0)
	    rs = mkfnamesuf1(mlfname,mailfname,"lock") ;

	if (rs >= 0)
	    rs = u_stat(mailfname,&sf) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progspool: u_stat() rs=%d size=%u\n",rs,sf.st_size) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	if (sf.st_size == 0)
	    goto ret0 ;

/* note that only *real*-user access is check here */

#if	CF_ACCESS
	rs = u_access(mailfname,pm) ;
#else
	rs = perm(mailfname,pip->uid,pip->gid,NULL,pm) ;
#endif /* CF_ACCESS */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progspool: perm() rs=%d\n",rs) ;
#endif
	if (rs < 0) goto ret0 ;

/* go to the end ... */

	u_seek(mfd,0L,SEEK_END) ;

	u_tell(mfd,&offset) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progspool: before for\n") ;
#endif

	starttime = time(NULL) ;

	for (i = 0 ; i < TRIES_OUTER ; i += 1) {

/* lock the mail spool file */

	    mip->daytime = time(NULL) ;

	    if ((mip->daytime - starttime) > pip->to_mailspool) {
	        rs = SR_TXTBSY ;
	        break ;
	    }

	    mip->lilen = mklockinfo(pip,mip->libuf,LIBUFLEN,mip->daytime) ;

	    rs = getspool_inner(pip,mip) ;
	    tlen = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progspool: getspool_inner() rs=%d\n",rs) ;
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
	        debugprintf("progspool: bottom for\n") ;
#endif

	} /* end for (outer) */

/* finish up and get out! */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progspool: finish rs=%d i=%d\n",
	        rs,i) ;
#endif

	if (i >= TRIES_OUTER)
	    rs = SR_TXTBSY ;

/* get out */
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progspool: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (progspool) */


/* local subroutines */


static int getspool_inner(pip,mip)
struct proginfo	*pip ;
struct modinfo	*mip ;
{
	SIGBLOCK	blocker ;
	int		rs ;
	int		rs1 ;
	int		tlen = 0 ;

	if ((rs = sigblock_start(&blocker,sigblocks)) >= 0) {
	    struct ustat	sb ;
	    int			lfd ;
	    int			j ;
	    const char		*mlfname = mip->mlfname ;
	    const char		*mailfname = mip->mailfname ;
	    char		timebuf[TIMEBUFLEN + 1] ;

	    for (j = 0 ; j < TRIES_INNER ; j += 1) {

	        rs = maillock_create(pip,mlfname,&mip->mdsb) ;
	        lfd = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("getspool_inner: maillock_create() rs=%d\n",
	                rs) ;
#endif

	        if ((rs != SR_ACCES) && (rs != SR_TXTBSY))
	            break ;

/* lock capture failed: check if the current mail lock is too old */

	        if (u_stat(mlfname,&sb) >= 0) {

	            mip->daytime = time(NULL) ;

	            if ((mip->daytime - sb.st_mtime) > MAILLOCKAGE) {

	                rs = maillock_unlink(pip,mlfname,&mip->mdsb) ;
	                if (rs >= 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("getspool_inner: deleting lockfile "
	                            "date=%s\n",
	                            timestr_logz(sb.st_mtime,timebuf)) ;
#endif

	                    if (pip->open.logprog) {
	                        const char *fmt = "removed lock w/ date=%s" ;
	                        logfile_printf(&pip->lh,fmt,
	                            timestr_logz(sb.st_mtime,timebuf)) ;
	                    }

	                } /* end if (we successfully removed the lock file) */

	            } /* end if */

	        } /* end if (good 'stat') */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("getspool_inner: about to sleep on lock\n") ;
#endif

	        sleep(1) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("getspool_inner: woke up f/ sleep on lock\n") ;
#endif

	    } /* end for (locking with the "lock" type file) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("getspool_inner: FILE lock create rs=%d\n",lfd) ;
#endif

#ifdef	COMMENT
	    if (j >= TRIES_INNER)
	        rs = SR_TXTBSY ;
#endif

/* if we have the mail lock, act on it */

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("getspool_inner: we got mail FILE lock\n") ;
#endif

/* put our name and PID into the lockfile */

	        if ((rs = u_write(lfd,mip->libuf,mip->lilen)) >= 0) {
	            uc_fsync(lfd) ;

/* try to open the mail spool file */

	            rs = mailcopy(pip,mailfname,TO_RECLOCK,mip->flp,mip->mfd) ;
	            tlen = rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("getspool_inner: mailcopy() rs=%d\n",rs) ;
#endif

	        } /* end if */

/* unlock mail spool area by removing the lock file */

	        u_close(lfd) ;

	        rs1 = maillock_unlink(pip,mlfname,&mip->mdsb) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (of handling new mail) */

/* turn interrupts, etc back on */

	    sigblock_finish(&blocker) ;
	} /* end if (sigblock) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("getspool_inner: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (getspool_inner) */


#ifdef	COMMENT
/* this subroutine both locks and unlocks! (depending on 'cmd') */
static int ourlock(fd,cmd,timeout)
int	fd, cmd, timeout ;
{
	int		rs ;
	int		i ;

	if (timeout < 0)
	    timeout = 100 ;

	if ((rs = u_seek(fd,0L,SEEK_SET)) >= 0) {
	    switch (cmd) {
	    case F_LOCK:
	    case F_TLOCK:
	        for (i = 0 ; i < timeout ; i += 1) {
	            rs = u_lockf(fd,F_TLOCK,0L) ;
	            if (rs >= 0) break ;
	            sleep(1) ;
	        } /* end for */
	        break ;
	    case F_ULOCK:
	        rs = u_lockf(fd,F_ULOCK,0L) ;
	        break ;
	    default:
	        rs = SR_INVAL ;
	        break ;
	    } /* end switch */
	} /* end if (file seekable) */

	return rs ;
}
/* end subroutine (ourlock) */
#endif /* COMMENT */


static int maillock_create(pip,mlfname,maildir_sbp)
struct proginfo	*pip ;
const char	mlfname[] ;
struct ustat	*maildir_sbp ;
{
	struct ustat	sb ;
	int		rs ;
	int		lfd = -1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("maillock_create: ent\n") ;
#endif

/* blow out if the lock file is already there! */

	rs = u_stat(mlfname,&sb) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("maillock_create: maillock u_stat() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = SR_TXTBSY ;
	    goto ret0 ;
	}

/* go ahead and try to create the lock file */

	rs = SR_ACCES ;
	if (pip->f.setgid) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("maillock_create: setting EGID\n") ;
#endif

	    u_setegid(pip->egid) ;

	    rs = u_creat(mlfname,0444) ;
	    lfd = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("maillock_create: rs=%d\n",rs) ;
#endif

	    u_setegid(pip->gid) ;

	}

	if (rs == SR_ACCES) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("maillock_create: no access\n") ;
#endif

	    rs = u_creat(mlfname,0444) ;
	    lfd = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("maillock_create: rs=%d\n",rs) ;
#endif

	}

	if (pip->euid == 0) {

	    if (rs == SR_ACCES) {

	        u_setegid(maildir_sbp->st_gid) ;

	        rs = u_creat(mlfname,0444) ;
	        lfd = rs ;

	        u_setegid(pip->gid) ;

	    }

	    if (rs == SR_ACCES) {

	        u_seteuid(maildir_sbp->st_uid) ;

	        rs = u_creat(mlfname,0444) ;
	        lfd = rs ;

	        u_seteuid(pip->uid) ;

	    }

	} /* end if (playing as super user) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("maillock_create: ret rs=%d lfd=%u\n",rs,lfd) ;
#endif

	return (rs >= 0) ? lfd : rs ;
}
/* end subroutine (maillock_create) */


static int maillock_unlink(pip,mlfname,maildir_sbp)
struct proginfo	*pip ;
const char	mlfname[] ;
struct ustat	*maildir_sbp ;
{
	int		rs = SR_ACCES ;

	if (pip->f.setgid) {
	    u_setegid(pip->egid) ;
	    rs = u_unlink(mlfname) ;
	    u_setegid(pip->gid) ;
	}

	if (rs == SR_ACCES)
	    rs = u_unlink(mlfname) ;

	if (pip->euid == 0) {

	    if (rs == SR_ACCES) {
	        u_setegid(maildir_sbp->st_gid) ;
	        rs = u_unlink(mlfname) ;
	        u_setegid(pip->gid) ;
	    }

	    if (rs == SR_ACCES) {
	        u_seteuid(maildir_sbp->st_uid) ;
	        rs = u_unlink(mlfname) ;
	        u_seteuid(pip->uid) ;
	    }

	} /* end if (trying as superuser) */

	return rs ;
}
/* end subroutine (maillock_unlink) */


static int mailcopy(pip,mailfname,to,flp,mfd)
struct proginfo	*pip ;
const char	mailfname[] ;
int		to ;
vecstr		*flp ;
int		mfd ;
{
	bfile		mfile ;
	int		rs ;
	int		ll ;
	int		tlen = 0 ;
	int		f_bol, f_eol ;
	int		f_leading = TRUE ;
	int		f_forwarded ;
	int		f_trunc = (! pip->f.nodel) ;

	if ((rs = bopen(&mfile,mailfname,"rw",0666)) >= 0) {
	    if ((rs = bcontrol(&mfile,BC_LOCK,to)) >= 0) {
		offset_t	off = 0 ;
		offset_t	off_mail = 0 ;
		const int	llen = LINEBUFLEN ;
	        const int	fl = MAILSPOOL_FORWARDEDLEN ;
		int		cl ;
	        const char	*fp = MAILSPOOL_FORWARDED ;
		const char	*cp ;
		char		lbuf[LINEBUFLEN + 1] ;

/* read out the spool file data (the new mail) */

	        f_bol = TRUE ;
	        while ((rs = breadline(&mfile,lbuf,llen)) > 0) {
	            ll = rs ;

	            f_eol = (lbuf[ll - 1] == '\n') ;

	            f_forwarded = FALSE ;
	            if (f_bol && f_leading && (ll >= fl))
	                f_forwarded = (strncmp(lbuf,fp,fl) == 0) ;

	            if (f_forwarded) {

	                if (flp != NULL) {
	                    cp = (fp + fl) ;
	                    cl = ll - fl ;
	                    rs = mailcopy_forwarded(flp,cp,cl) ;
	                }

	            } else {

	                if (tlen == 0)
	                    off_mail = off ;

	                f_leading = FALSE ;
	                rs = uc_writen(mfd,lbuf,ll) ;
	                tlen += rs ;

	            } /* end if */

	            f_bol = f_eol ;
	            off += ll ;
	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && (tlen > 0) && f_trunc) {
	            rs = bcontrol(&mfile,BC_TRUNCATE,off_mail) ;
	        }

	    } /* end if (lock) */
	    bclose(&mfile) ;
	} /* end if (open-output-file) */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (mailcopy) */


static int mailcopy_forwarded(flp,sp,sl)
vecstr		*flp ;
const char	*sp ;
int		sl ;
{
	int		rs = SR_OK ;
	int		n = 0 ;
	int		cl ;
	const char	*tp, *cp ;

	if ((sl > 0) && (sp[sl - 1] == '\n')) sl -= 1 ;

	while ((tp = strnchr(sp,sl,',')) != NULL) {
	    if ((cl = nextfield(sp,(tp - sp),&cp)) > 0) {
	        n += 1 ;
	        rs = vecstr_add(flp,cp,cl) ;
	    }
	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    if ((cl = nextfield(sp,sl,&cp)) > 0) {
	        n += 1 ;
	        rs = vecstr_add(flp,cp,cl) ;
	    }
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mailcopy_forwarded) */


/* make information to put into the lock file */
static int mklockinfo(PROGINFO *pip,char *rbuf,int rlen,time_t dt)
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    pid_t	pid = (pip->pid >= 0) ? pip->pid : 0 ;
	    const char	*ct = ((pip->f.sysv_ct) ? "SYSV" : "BSD") ;
	    char	tbuf[TIMEBUFLEN + 1] ;

	    sbuf_printf(&b,"%u\n",(uint) pid) ;

	    sbuf_printf(&b,"%s!%s\n",
	        pip->nodename,pip->username) ;

	    timestr_logz(dt,tbuf),
	    sbuf_printf(&b,"%s %s%s/%s\n",tbuf,pip->progname,VERSION,ct) ;

	    sbuf_printf(&b,"logid=%s\n",pip->logid) ;

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (mklockinfo) */


