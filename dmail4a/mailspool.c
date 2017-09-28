/* mailspool */

/* open a mail-spool file w/ its associated lock */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to deliver new mail to the mail spool file for
	a given recipient.

	Synopsis:

	int mailspool_open(op,md,un,of,om,to)
	MAILSPOOL	*op ;
	cchar		md[] ;
	cchar		un[] ;
	int		of ;
	mode_t		om ;
	int		to ;

	Arguments:

	op		object pointer
	md		mail directory
	un		username
	of		open-flags
	om		open-mode
	to		time-out

	Returns:

	>=0		dile-descriptor
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<estrings.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<localmisc.h>

#include	"mailspool.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */

#define	TO_SPOOL	(10*60)

#define	SUBINFO		struct subinfo


/* external subroutines */

extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	msleep(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct subinfo {
	MAILSPOOL	*op ;
	cchar		*md ;
	cchar		*un ;
	cchar		*mfname ;
	gid_t		egid ;
	mode_t		om ;
	int		of ;
	int		to ;
	int		f_create:1 ;
} ;


/* forward references */

static int	mailspool_lfbegin(MAILSPOOL *,cchar *,cchar *) ;
static int	mailspool_lfend(MAILSPOOL *) ;

static int	subinfo_start(SUBINFO *,MAILSPOOL *,cchar *,cchar *,int,
			mode_t,int) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_checkcreate(SUBINFO *) ;
static int	subinfo_trying(SUBINFO *) ;
static int	subinfo_lockoutbegin(SUBINFO *,time_t) ;
static int	subinfo_lockoutend(SUBINFO *,int) ;
static int	subinfo_lockcreate(SUBINFO *) ;
static int	subinfo_lockin(SUBINFO *) ;
static int	subinfo_minmod(SUBINFO *,int,mode_t) ;
static int	subinfo_chown(SUBINFO *) ;

static int	getlockcmd(int) ;


/* local variables */


/* exported subroutines */


int mailspool_open(MAILSPOOL *op,cchar *md,cchar *un,int of,mode_t om,int to)
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		mfd = -1 ;

#if	CF_DEBUGS
	debugprintf("mailspool_open: ent\n") ;
	debugprintf("mailspool_open: md=%s\n",md) ;
	debugprintf("mailspool_open: un=%s\n",un) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (md == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	of |= O_LARGEFILE ;
	if ((rs = subinfo_start(sip,op,md,un,of,om,to)) >= 0) {
	    if ((rs = mailspool_lfbegin(op,md,un)) >= 0) {
	        if ((rs = subinfo_checkcreate(sip)) >= 0) {
	            if ((rs = subinfo_trying(sip)) >= 0) {
		        op->magic = MAILSPOOL_MAGIC ;
		    }
	        }
		if (rs < 0)
		    mailspool_lfend(op) ;
	    } /* end if (mailspool-lf) */
	    mfd = subinfo_finish(sip) ;
	    if (rs >= 0) rs = mfd ;
	} /* end if (subinfo) */
	if (rs < 0) {
	    mailspool_close(op) ;
	}

#if	CF_DEBUGS
	debugprintf("mailspool_open: ret rs=%d mfd=%u\n",rs,mfd) ;
#endif

	return (rs >= 0) ? mfd : rs ;
}
/* end subroutine (mailspool_open) */


int mailspool_close(MAILSPOOL *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != MAILSPOOL_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("mailspool_close: ent\n") ;
#endif

	if (op->mfd >= 0) {
	    rs1 = u_close(op->mfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mfd = -1 ;
	} 

	if (op->lfname != NULL) {
	    if ((op->lfname[0] != '\0') && op->f_created) {
	        rs1 = uc_unlink(op->lfname) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = mailspool_lfend(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("mailspool_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mailspool_close) */


static int mailspool_lfbegin(MAILSPOOL *op,cchar *md,cchar *un)
{
	int		rs ;
	const int	nlen = MAXNAMELEN ;
	char		nbuf[MAXNAMELEN+1] ;
	if ((rs = sncpy2(nbuf,nlen,un,".lock")) >= 0) {
	    char	lfname[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(lfname,md,nbuf)) >= 0) {
		cchar	*cp ;
		if ((rs = uc_mallocstrw(lfname,rs,&cp)) >= 0) {
		    op->lfname = cp ;
		}
	    }
	}
	return rs ;
}
/* end subroutine (mailspool_lfbegin) */


static int mailspool_lfend(MAILSPOOL *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->lfname != NULL) {
	    rs1 = uc_free(op->lfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->lfname = NULL ;
	}
	return rs ;
}
/* end subroutine (mailspool_lfend) */


int mailspool_setlockinfo(MAILSPOOL *op,cchar *wbuf,int wlen)
{
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (wbuf == NULL) return SR_FAULT ;
#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_setlockinfo: ent wlen=%d\n",wlen) ;
#endif
	if (op->magic != MAILSPOOL_MAGIC) return SR_NOTOPEN ;

	if (wlen < 0) wlen = strlen(wbuf) ;

	if ((rs = u_open(op->lfname,O_WRONLY,0666)) >= 0) {
	    const int	fd = rs ;
	    rs = u_write(fd,wbuf,wlen) ;
	    rs1 = u_close(fd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file) */

#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_setlockinfo: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mailspool_setlockinfo) */


/* private subroutines */


static int subinfo_start(sip,op,md,un,of,om,to)
SUBINFO		*sip ;
MAILSPOOL	*op ;
cchar 		*md ;
cchar		*un ;
int		of ;
mode_t		om ;
int		to ;
{
	int		rs ;
	char		mfname[MAXPATHLEN+1] ;
	memset(sip,0,sizeof(SUBINFO)) ;
	sip->op = op ;
	sip->md = md ;
	sip->un = un ;
	sip->of = of ;
	sip->om = om ;
	sip->to = to ;
	sip->egid = getegid() ;
	if ((rs = mkpath2(mfname,md,un)) >= 0) {
	    cchar	*cp ;
	    if ((rs = uc_mallocstrw(mfname,rs,&cp)) >= 0) {
		sip->mfname = cp ;
	    } /* end if (m-a) */
	} /* end if (mkpath) */
	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	MAILSPOOL	*op = sip->op ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		mfd  ;
	if (sip == NULL) return SR_FAULT ;
	if (sip->mfname != NULL) {
	    rs1 = uc_free(sip->mfname) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->mfname = NULL ;
	}
	mfd = op->mfd ;
	return (rs >= 0) ? mfd : rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_checkcreate(SUBINFO *sip)
{
	struct ustat	sb ;
	int		rs ;
	cchar		*mfname = sip->mfname ;
	if ((rs = u_stat(mfname,&sb)) >= 0) {
	    if (! S_ISREG(sb.st_mode)) {
		rs = SR_ISDIR ;
	    }
	} else if (isNotPresent(rs)) {
	    const int	of = sip->of ;
	    if (of&O_CREAT) {
		rs = SR_OK ;
		sip->f_create = TRUE ;
	    }
	}
#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_checkcreate: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_checkcreate) */


static int subinfo_trying(SUBINFO *sip)
{
	time_t		ti_start = time(NULL) ;
	time_t		ti_now ;
	int		rs = SR_OK ;
	int		mfd = -1 ;
	int		rs1 ;
	ti_now = ti_start ;
	while (rs >= 0) {
	    if ((rs = subinfo_lockoutbegin(sip,ti_now)) > 0) {
		rs = subinfo_lockin(sip) ;
		mfd = rs ;
	        rs1 = subinfo_lockoutend(sip,rs) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (subinfo-lockout) */
	    if (mfd >= 0) break ;
	    if (rs == SR_AGAIN) rs = SR_OK ;
	    if (rs >= 0) sleep(1) ;
	    ti_now = time(NULL) ;
	    if ((ti_now-ti_start) >= sip->to) rs = SR_AGAIN ;
	} /* end while */
	return (rs >= 0) ? mfd : rs ;
}
/* end subroutine (subinfo_trying) */


static int subinfo_lockoutbegin(SUBINFO *sip,time_t dt)
{
	MAILSPOOL	*op = sip->op ;
	struct ustat	sb ;
	int		rs ;
	int		f = FALSE ;
	cchar		*lfname ;
	lfname = op->lfname ;
#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_lockoutbegin: lfname=%s\n",lfname) ;
#endif
	if ((rs = u_stat(lfname,&sb)) >= 0) {
#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_lockoutbegin: already\n") ;
#endif
	    if ((dt-sb.st_mtime) >= TO_SPOOL) {
#if	CF_DEBUGS
		debugprintf("mailspool/subinfo_lockoutbegin: old unlink\n") ;
#endif
		if ((rs = u_unlink(lfname)) >= 0) {
		    rs = subinfo_lockcreate(sip) ;
		    f = rs ;
		}
	    }
	} else if (isNotAccess(rs)) {
#if	CF_DEBUGS
	    debugprintf("mailspool/subinfo_lockoutbegin: not-already\n") ;
#endif
	    rs = subinfo_lockcreate(sip) ;
	    f = rs ;
	}

#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_lockoutbegin: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_lockoutbegin) */


static int subinfo_lockoutend(SUBINFO *sip,int prs)
{
	MAILSPOOL	*op = sip->op ;
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_lockoutend: ent prs=%d\n",prs) ;
#endif
	if (prs < 0) {
	    char	*bp = (char *) op->lfname ;
	    rs1 = uc_unlink(op->lfname) ;
	    if (rs >= 0) rs = rs1 ;
	    bp[0] = '\0' ;
	}
#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_lockoutend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_lockoutend) */


static int subinfo_lockcreate(SUBINFO *sip)
{
	MAILSPOOL	*op = sip->op ;
	const int	of = (O_RDWR|O_CREAT|O_EXCL) ;
	int		rs ;
	if ((rs = u_open(op->lfname,of,0664)) >= 0) {
	    op->f_created = TRUE ;
	    u_close(rs) ;
	    rs = 1 ;
	} else if (isNotAccess(rs)) {
	    rs = SR_OK ;
	}
#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_lockcreate: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_lockcreate) */


static int subinfo_lockin(SUBINFO *sip)
{
	MAILSPOOL	*op = sip->op ;
	const mode_t	om = sip->om ;
	const int	of = sip->of ;
	int		rs ;
	int		mfd = -1 ;
	cchar		*mfname = sip->mfname ;
#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	debugprintf("mailspool/subinfo_lockin: ent mf=%s\n",mfname) ;
	debugprintf("mailspool/subinfo_lockin: of=%s\n",obuf) ;
	debugprintf("mailspool/subinfo_lockin: om=%05o\n",om) ;
	}
#endif /* CF_DEBUGS */
	if ((rs = u_open(mfname,of,om)) >= 0) {
	    mfd = rs ;
#if	CF_DEBUGS
	    debugprintf("mailspool/subinfo_lockin: u_open() rs=%d\n",rs) ;
#endif
	    if ((rs = getlockcmd(of)) >= 0) {
		const int	cmd = rs ;
#if	CF_DEBUGS
		debugprintf("mailspool/subinfo_lockin: getlockcmd() rs=%d\n",
			rs) ;
#endif
	        if ((rs = uc_lockfile(mfd,cmd,0L,0,1)) >= 0) {
		    if ((rs = subinfo_minmod(sip,mfd,om)) >= 0) {
		        if ((rs = subinfo_chown(sip)) >= 0) {
		            op->mfd = mfd ;
		        }
		    }
		} /* end if (lock) */
#if	CF_DEBUGS
		debugprintf("mailspool/subinfo_lockin: uc_lockfile-out rs=%d\n",
			rs) ;
#endif
	    } /* end if (getlockcmd) */
	    if (rs < 0) {
		u_close(mfd) ;
		op->mfd = -1 ;
	    }
	} /* end if (file-open) */
#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_lockin: ret rs=%d mfd=%d\n",rs,mfd) ;
#endif
	return (rs >= 0) ? mfd : rs ;
}
/* end subroutine (subinfo_lockin) */


static int subinfo_minmod(SUBINFO *sip,int mfd,mode_t om)
{
	int		rs = SR_OK ;
	if (sip->f_create) {
	    rs = uc_fminmod(mfd,om) ;
	}
#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_minmod: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_minmod) */


static int subinfo_chown(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (sip->f_create) {
	    struct ustat	sb ;
	    const gid_t		egid = sip->egid ;
	    cchar		*md = sip->md ;
	    if ((rs = u_stat(md,&sb)) >= 0) {
	        struct passwd	pw ;
		const gid_t	gid = sb.st_gid ;
	        const int	pwlen = getbufsize(getbufsize_pw) ;
	        char		*pwbuf ;
	        if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
		    if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,sip->un)) >= 0) {
			const uid_t	euid = geteuid() ;
		        const uid_t	uid = pw.pw_uid ;
			if ((uid != euid) || (gid != egid)) {
			    const int	cv = _PC_CHOWN_RESTRICTED ;
	                    if ((rs = u_pathconf(md,cv,NULL)) == 0) {
			        cchar	*mfname = sip->mfname ;
			        f = TRUE ;
    		                rs = u_chown(mfname,uid,gid) ;
	                    } else if (rs == SR_NOSYS) {
	                        rs = SR_OK ;
	                    }
			} /* end if (needed) */
		    } else if (isNotPresent(rs)) {
		        rs = SR_OK ;
		    }
		    uc_free(pwbuf) ;
	        } /* end if (m-a) */
	    } /* end if (stat) */
	} /* end if (file was created) */
#if	CF_DEBUGS
	debugprintf("mailspool/subinfo_chown: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_chown) */


static int getlockcmd(int of)
{
	const int	am = (of&O_ACCMODE) ;
	int		cmd = SR_INVALID ;
	switch (am) {
	case O_WRONLY:
	case O_RDWR:
	    cmd = F_WLOCK ;
	    break ;
	case O_RDONLY:
	    cmd = F_RLOCK ;
	    break ;
	} /* end switch */
	return cmd ;
}
/* end subroutine (getlockcmd) */


