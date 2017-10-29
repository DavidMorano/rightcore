/* progboxer */

/* delivers mail messages (data) to a mailbox */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug prints */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */
#define	CF_TESTSLEEP	0		/* test sleep mode */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to deliver mail to a mailbox file for
	a given recipient.  That file is located in the mail directory
	area of the given recipient user.

	Synopsis:

	int progboxer(pip,tfd,rp)
	PROGINFO	*pip ;
	int		tfd ;
	RECIP		*rp ;

	Arguments:

	pip		program information pointer
	tfd		file descriptor (FD) to target mailbox file
	rp		pointer to recipient list container

	Reuturns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<ugetpw.h>
#include	<getax.h>
#include	<sbuf.h>
#include	<bfile.h>
#include	<sigblock.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"recip.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */

#ifndef	BUFLEN
#define	BUFLEN		256
#endif

#define	TRIES_OUTER	20
#define	TRIES_INNER	4

#define	MAILFILEMODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#ifndef	S_ISLNK
#define	S_ISLNK(mode)	((mode) & S_IFLNK)
#endif

#ifndef	OPENS
#define	OPENS		struct opens
#endif


/* external subroutines */

extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	recipcopyparts(RECIP *,int,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isOneOf(const int *,int) ;

extern int	locinfo_mboxget(LOCINFO *,int,cchar **) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct opens {
	uint		f_needsid:1 ;	/* cache for need set-ID next time */
} ;


/* forward references */

static int	progboxer_folder(PROGINFO *,cchar *) ;
static int	boxadd(PROGINFO *,int,RECIP *,cchar *) ;
static int	openspoolfile(PROGINFO *,OPENS *,cchar *) ;
static int	createspoolfile(PROGINFO *,OPENS *,cchar *,int) ;
static int	ourlock(int,int,int) ;
static int	mkboxdir(PROGINFO *,cchar *) ;
static int	opens_init(OPENS *) ;


/* local variables */

static const int	oursigs[] = {
	    SIGALRM,
	    SIGPOLL,
	    SIGHUP,
	    SIGTERM,
	    SIGINT,
	    SIGQUIT,
	    SIGWINCH,
	    SIGURG,
	    0
} ;

static const int rslocked[] = {
	    SR_ACCES,
	    SR_AGAIN,
	    0
} ;


/* exported subroutines */


int progboxer(PROGINFO *pip,int tfd,RECIP *rp)
{
	struct passwd	pw ;
	OPENS		os ;
	LOCINFO		*lip = pip->lip ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	int		rs1 ;
	int		tlen = 0 ;
	char		*pwbuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progboxer: ent\n") ;
	    debugprintf("progboxer: euid=%d egid=%d\n",pip->euid,pip->egid) ;
	    debugprintf("progboxer: xeuid=%d xegid=%d\n",geteuid(),getegid()) ;
	}
#endif

	if (rp == NULL) return SR_FAULT ;

	if (rp->recipient[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progboxer: recip=%s\n",rp->recipient) ;
#endif

	opens_init(&os) ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,rp->recipient)) >= 0) {
	        cchar	*homedname = pw.pw_dir ;
	        cchar	*folder = pip->boxdname ;
	        char	mailfname[MAXPATHLEN + 1] ;
	        if ((rs = mkpath2(mailfname,homedname,folder)) >= 0) {
	            const int	plen = rs ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progboxer: mid rs=%d mailfname=%s\n",
			    rs,mailfname) ;
#endif
	            if ((rs = progboxer_folder(pip,mailfname)) >= 0) {
	                cchar	*np ;
	                int	i ;
	                for (i = 0 ; locinfo_mboxget(lip,i,&np) >= 0 ; i += 1) {
			    if (np != NULL) {
	                        if ((rs = pathadd(mailfname,plen,np)) >= 0) {
	                            rs = boxadd(pip,tfd,rp,mailfname) ;
	                            tlen += rs ;
				}
	                    }
	                    if (rs < 0) break ;
	                } /* end for */
	                rs1 = recip_ds(rp,rs) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (progboxer_folder) */
	        } /* end if (mkpath) */
	    } /* end if (getpw_name) */
	    uc_free(pwbuf) ;
	} /* end if (m-a) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progboxer: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (progboxer) */


/* local subroutines */


static int progboxer_folder(PROGINFO *pip,cchar *mailfname)
{
	struct ustat	sb ;
	const int	nrs = SR_NOTFOUND ;
	int		rs ;
	if ((rs = u_stat(mailfname,&sb)) >= 0) {
	    if (! S_ISDIR(sb.st_mode)) rs = SR_NOTDIR ;
	} else if (rs == nrs) {
	    rs = mkboxdir(pip,mailfname) ;
	}
	return rs ;
}
/* end subroutine (progboxer_folder) */


static int boxadd(PROGINFO *pip,int tfd,RECIP *rp,cchar *mailfname)
{
	OPENS		os ;
	uid_t		uid_recip = -1 ;
	int		rs ;
	int		tlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progboxer/boxadd: ent mfn=%s\n",mailfname) ;
#endif

	opens_init(&os) ;

	if ((rs = u_rewind(tfd)) >= 0) {
	    int		sfd ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        struct ustat	sb ;
	        debugprintf("progboxer: tfd u_seek() rs=%d\n",rs) ;
	        u_fstat(tfd,&sb) ;
	        debugprintf("progboxer: tfd size=%ld\n",sb.st_size) ;
	    }
#endif

/* try to open the mail spool file */

	    rs = openspoolfile(pip,&os,mailfname) ;
	    sfd = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progboxer: openspoolfile() rs=%d\n",rs) ;
#endif

	    if (rs == SR_NOENT) {
	        rs = createspoolfile(pip,&os,mailfname,uid_recip) ;
	        sfd = rs ;
	    } /* end if (could not open mailfile) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progboxer: open spool file rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        struct ustat	sb ;
	        if ((rs = u_fstat(sfd,&sb)) >= 0) {
	            if (S_ISREG(sb.st_mode)) {
	                rs = u_seek(sfd,0L,SEEK_END) ;
	            }
	        }
	    }

	    if (rs >= 0) {

	        if ((rs = ourlock(sfd,F_LOCK,pip->to_spool)) >= 0) {
	            SIGBLOCK	blocks ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progboxer: got the RECORD lock\n") ;
#endif

/* do the copy atomically */

	            if ((rs = sigblock_start(&blocks,oursigs)) >= 0) {
			offset_t	soff ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_TESTSLEEP
	                sleep(20) ;
#endif

	                u_tell(sfd,&soff) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progboxer: soff=%llu\n",
	                        soff) ;
#endif

/* do the actual copy according to the specification on this recipient */

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progboxer: recipcopyparts()\n") ;
#endif

	                rs = recipcopyparts(rp,tfd,sfd) ;
	                tlen += rs ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progboxer: recipcopyparts() rs=%d\n",
	                        rs) ;
#endif

	                if (rs < 0)
	                    uc_ftruncate(sfd,soff) ;

	                uc_fsync(sfd) ;

/* turn interrupts, etc back on */

	                sigblock_finish(&blocks) ;
	            } /* end if (sigblock) */

/* unlock the mailbox file */

	            ourlock(sfd,F_ULOCK,pip->to_spool) ;
	        } /* end if (we got the record lock) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progboxer: u_close() rs=%d tlen=%u\n",
	                rs,tlen) ;
#endif

	        u_close(sfd) ;
	    } /* end if (open or created the mail spool file) */

	} /* end if (rewind) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progboxer: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (boxadd) */


/* open the spool file (using SUID or SGID as necessary) */
static int openspoolfile(pip,osp,mailfname)
PROGINFO	*pip ;
OPENS		*osp ;
cchar		mailfname[] ;
{
	uid_t		euid ;
	gid_t		egid ;
	const int	oflags = (O_RDWR | O_APPEND) ;
	int		rs = SR_ACCESS ;
	int		rs1 ;
	int		f_suid = FALSE ;
	int		f_sgid = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progboxer/openspoolfile: ent mf=%s\n",mailfname) ;
#endif

	if (! osp->f_needsid) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("progboxer/openspoolfile: 1st open mailfname=%s\n",
	            mailfname) ;
	        debugprintf("progboxer/openspoolfile: euid=%d egid=%d\n",
	            geteuid(),getegid()) ;
	    }
#endif
	    rs = uc_open(mailfname,oflags,0666) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progboxer/openspoolfile: "
			"1st uc_open() rs=%d\n",rs) ;
#endif
	} /* end if (trying without using some sort of SID) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progboxer/openspoolfile: mid rs=%d\n",rs) ;
#endif

	if (rs == SR_ACCESS) {

	    if (pip->f.setuid) {
	        euid = geteuid() ;
	        if (euid != pip->euid) {
	            rs1 = u_seteuid(pip->euid) ;
	            f_suid = (rs1 >= 0) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("progboxer/openspoolfile: "
			    "u_seteuid() rs=%d euid=%d\n",
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
	                debugprintf("progboxer/openspoolfile: "
			    "u_setegid() rs=%d egid=%d\n",
	                    rs1,getegid()) ;
#endif
	        }
	    } /* end if (we have SGID) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("progboxer/openspoolfile: 2nd open \n") ;
	        debugprintf("progboxer/openspoolfile: euid=%d egid=%d\n",
	            geteuid(),getegid()) ;
	    }
#endif

	    if (f_suid || f_sgid) {
	        rs = uc_open(mailfname,oflags,0666) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progboxer/openspoolfile: uc_open() rs=%d\n",
			rs) ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progboxer/openspoolfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (openspoolfile) */


static int createspoolfile(pip,osp,mailfname,uid_recip)
PROGINFO	*pip ;
OPENS		*osp ;
cchar		mailfname[] ;
int		uid_recip ;
{
	struct ustat	sb ;
	uid_t		euid ;
	gid_t		egid ;
	int		rs = SR_ACCESS ;
	int		rs1 ;
	int		sfd ;
	int		f_suid = FALSE ;
	int		f_sgid = FALSE ;

	if (! osp->f_needsid) {

	    rs = u_creat(mailfname,0666) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progboxer: u_creat() rs=%d\n",rs) ;
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
	    mode_t	m ;

	    rs1 = u_fstat(sfd,&sb) ;

	    if ((rs1 >= 0) && ((sb.st_mode & MAILFILEMODE) != MAILFILEMODE)) {
	        m = sb.st_mode | MAILFILEMODE ;
	        u_fchmod(sfd,m) ;
	    }

	    u_fchown(sfd,uid_recip,pip->gid_mail) ;

	} /* end if (needed to create mailfile) */

	return (rs >= 0) ? sfd : rs ;
}
/* end subroutine (createspoolfile) */


static int mkboxdir(PROGINFO *pip,cchar boxdname[])
{
	uid_t		euid ;
	gid_t		egid ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_suid = FALSE ;
	int		f_sgid = FALSE ;

	if (pip->f.setuid) {
	    euid = geteuid() ;
	    if (euid != pip->uid) {
	        rs1 = u_seteuid(pip->uid) ;
	        f_suid = (rs1 >= 0) ;
	    }
	} /* end if (we have SUID) */

	if (pip->f.setgid) {
	    egid = getegid() ;
	    if (egid != pip->gid) {
	        rs1 = u_setegid(pip->gid) ;
	        f_sgid = (rs1 >= 0) ;
	    }
	} /* end if (we have SGID) */

	rs = u_mkdir(boxdname,0775) ;
	if (f_suid) u_seteuid(euid) ;
	if (f_sgid) u_setegid(egid) ;

	return rs ;
}
/* end subroutine (mkboxdir) */


static int opens_init(OPENS *op)
{
	memset(op,0,sizeof(OPENS)) ;
	return SR_OK ;
}
/* end subroutine (opens_init) */


/* this subroutine both locks and unlocks! (depending on 'cmd') */
static int ourlock(int fd,int cmd,int timeout)
{
	int		rs ;
	int		i ;

	if (timeout < 0)
	    timeout = 100 ;

/* we're good, go! */

	switch (cmd) {
	case F_LOCK:
	case F_TLOCK:
	    for (i = 0 ; i < timeout ; i += 1) {
	        if (i > 0) sleep(1) ;
	        if ((rs = uc_lockf(fd,F_TLOCK,0L)) >= 0) break ;
	        if (! isOneOf(rslocked,rs)) break ;
	    } /* end for */
	    break ;
	case F_ULOCK:
	    rs = uc_lockf(fd,F_ULOCK,0L) ;
	    break ;
	default:
	    rs = SR_INVAL ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (ourlock) */


