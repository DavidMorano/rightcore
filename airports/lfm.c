/* lfm (Lock File Manager) */

/* manage file-lock operations */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We want to check that we still own the lock file and also update it by
        writting a current time into it.

	Synopsis:

	int lfm_start(op,fname,type,timeout,lcp,nn,un,banner)
	LFM		*op ;
	const char	fname[] ;
	int		type, timeout ;
	LFM_CHECK	*lcp ;
	const char	nn[], un[], banner[] ;

	Arguments:

	op		object pointer
	fname		fname of lock file
	type		type of lock file
	timeout		timeout on waiting for it
	lcp		optional pointer to lock check object
	nn		node name
	un		user name
	banner		program banner

	Returns:

	>=0	OK
	<0	error


*******************************************************************************/


#define	LFM_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<filebuf.h>
#include	<storeitem.h>
#include	<getxusername.h>
#include	<ugetpid.h>
#include	<localmisc.h>

#include	"lfm.h"


/* typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


/* local defines */

#define	LFM_LOCKINFO	struct lfm_lockinfo

#ifndef	USERNAMELEN
#ifndef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((NODENAMELEN + 1 + LOGNAMELEN + 1),2048)
#endif

#define	TO_LOCK		2
#define	TO_MINCHECK	3
#define	TO_MINSTAT	(60 + 3)

#if	CF_DEBUGS
#define	BUFLEN		80
#else
#define	BUFLEN		(MAXPATHLEN + 40)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snopenflags(char *,int,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	getnodename(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	format(char *,int,int,const char *,va_list) ;
extern int	sfnamecomp(cchar *,int,cchar **) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct lfm_lockinfo {
	const char	*nn ;
	const char	*un ;
	const char	*bn ;
	time_t		daytime ;
} ;


/* forward references */

static int	lfm_lockload(LFM *,LFM_CHECK *) ;
static int	lfm_locklost(LFM *,LFM_CHECK *,FILEBUF *) ;
static int	lfm_lockwrite(LFM *,LFM_LOCKINFO *,int) ;
static int	lfm_lockwriter(LFM *,LFM_LOCKINFO *,int) ;
static int	lfm_lockwritedate(LFM *,time_t) ;
static int	lfm_lockreadpid(LFM *) ;
static int	lfm_lockbegin(LFM *op) ;
static int	lfm_lockend(LFM *op) ;

static int	check_init(LFM_CHECK *) ;


/* local variables */

#if	CF_DEBUGS
static const char	*types[] = {
	"record",
	"create",
	"exclusive",
	NULL
} ;
#endif /* CF_DEBUGS */


/* exported subroutines */


int lfm_start(op,fname,type,to,lcp,nn,un,bn)
LFM		*op ;
const char	fname[] ;
int		type ;
int		to ;
LFM_CHECK	*lcp ;
const char	nn[], un[], bn[] ;
{
	struct ustat	sb ;
	const time_t	daytime = time(NULL) ;
	mode_t		omode ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;
	int		dnl ;
	int		fnl ;
	int		lfd ;
	int		oflags ;
	const char	*fnp ;
	const char	*dnp ;
	const char	*cp ;
	char		nodename[NODENAMELEN + 1] ;
	char		unbuf[USERNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("lfm_start: ent fname=%s\n",fname) ;
	debugprintf("lfm_start: type=%u\n",type) ;
#endif

	if (to < 0) to = LFM_TO ;

	memset(op,0,sizeof(LFM)) ;

/* argument check */

	if (fname[0] == '\0') return SR_INVALID ;

	if ((type < 0) || (type >= LFM_TOVERLAST))
	    return SR_INVALID ;

/* priliminary initialization */

	op->type = type ;
	op->lfd = -1 ;
	op->pid_lock = -1 ;
	op->tocheck = TO_MINCHECK ;
	op->tolock = TO_LOCK ;
	op->pid = ugetpid() ;

	if (lcp != NULL) check_init(lcp) ;

/* continue itialization */

	if ((rs >= 0) && (nn == NULL)) {
	    nn = nodename ;
	    rs = getnodename(nodename,NODENAMELEN) ;
	}

	if ((rs >= 0) && (un == NULL)) {
	    un = unbuf ;
	    rs = getusername(unbuf,USERNAMELEN,-1) ;
	}

	if (rs < 0) goto bad0 ;

/* create the LOCK directory if necessary */

	if ((fnl = sfnamecomp(fname,-1,&fnp)) > 0) {
	    if ((rs = uc_mallocstrw(fnp,fnl,&cp)) >= 0) {
	        op->lockfname = cp ;
	    }
	} else
	    rs = SR_INVALID ;

	if (rs < 0) goto bad1 ;

#if	CF_DEBUGS
	debugprintf("lfm_start: n=%s u=%s\n",nn,un) ;
	debugprintf("lfm_start: banner=%s\n",bn) ;
#endif

/* is there a directory part? (besides root!) */

	if ((dnl = sfdirname(op->lockfname,len,&dnp)) > 0) {
	    char	tmpdname[MAXPATHLEN+1] ;

	    if ((rs = mkpath1w(tmpdname,dnp,dnl)) >= 0) {
	        const int	am = (X_OK | W_OK) ;

#if	CF_DEBUGS
	        debugprintf("lfm_start: check directory access\n") ;
#endif

	        rs1 = u_access(tmpdname,am) ;

#if	CF_DEBUGS
	        debugprintf("lfm_start: u_access() rs=%d\n",rs1) ;
#endif

	        if ((rs1 == SR_NOENT) || (rs1 == SR_ACCESS)) {
	            rs = mkdirs(tmpdname,0777) ;
	        } else
	            rs = rs1 ;

	    } /* end if */

	} /* end if */
	if (rs < 0) goto bad5 ;

/* is there a file-lock already? */

	rs1 = u_stat(op->lockfname,&sb) ;

	if ((rs1 >= 0) && S_ISDIR(sb.st_mode)) {
	    rs = SR_ISDIR ;
	    goto bad5 ;
	}

	op->ti_check = daytime ;
	op->ti_stat = daytime ;
	if ((type >= LFM_TCREATE) && (rs1 >= 0)) {

	    if ((daytime - sb.st_mtime) > to) {

#if	CF_DEBUGS
	        debugprintf("lfm_start: deleting lockfname=%s\n",
	            op->lockfname) ;
#endif

	        u_unlink(op->lockfname) ;

	    } else {
	        if (lcp != NULL) lfm_lockload(op,lcp) ;
	        rs = SR_AGAIN ;	/* lock exists */
	        goto bad5 ;
	    }

	} /* end if (good stat) */

/* create the lock file */

#if	CF_DEBUGS
	debugprintf("lfm_start: type=%s(%u)\n", 
	    types[type],type) ;
#endif

	oflags = (O_RDWR | O_CREAT) ;
	omode = 0664 ;
	if (type >= LFM_TCREATE)
	    omode = 0444 ;

	if (type >= LFM_TEXCLUSIVE)
	    oflags |= O_EXCL ;

#if	CF_DEBUGS
	{
	    char	obuf[80] ;
	    snopenflags(obuf,80,oflags) ;
	    debugprintf("lfm_start: om=%s\n",obuf) ;
	    debugprintf("lfm_start: omode=%06o\n",omode) ;
	}
#endif /* CF_DEBUGS */

	rs = u_open(op->lockfname,oflags,omode) ;
	lfd = rs ;

#if	CF_DEBUGS
	debugprintf("lfm_start: u_open() rs=%d\n",rs) ;
#endif

	if (rs < 0) {
	    switch (rs) {
	    case SR_EXIST:
	    case SR_ACCESS:
	        rs = SR_LOCKED ;
	        break ;
	    } /* end switch */
	    if (lcp != NULL) lfm_lockload(op,lcp) ;
	    goto bad5 ;
	} /* end if */

/* capture the lock (if we can) */

	rs = uc_lockf(lfd,F_TLOCK,0L) ;
#if	CF_DEBUGS
	debugprintf("lfm_start: u_lockf() rs=%d\n",rs) ;
#endif
	if (rs == SR_ACCESS) rs = SR_LOCKED ;

	if (rs < 0) {
	    if (lfd >= 0) {
	        u_close(lfd) ;
	        lfd = -1 ;
	    }
	    if (lcp != NULL) lfm_lockload(op,lcp) ;
	    goto bad5 ;
	}

#if	CF_DEBUGS
	debugprintf("lfm_start: we captured the file-lock\n") ;
#endif

	{
	    LFM_LOCKINFO	li ;
	    li.daytime = daytime ;
	    li.nn = nn ;
	    li.un = un ;
	    li.bn = bn ;
	    if ((rs = lfm_lockwrite(op,&li,lfd)) >= 0) {
	        if ((rs = u_fstat(lfd,&sb)) >= 0) {
	            op->lfd = lfd ;
	            op->magic = LFM_MAGIC ;
	            op->dev = sb.st_dev ;
	            op->ino = sb.st_ino ;
	        }
	    }
	}

	if (rs < 0)
	    goto bad6 ;

#if	CF_DEBUGS
	debugprintf("lfm_start: lfd=%u\n",lfd) ;
#endif

ret0:

#if	CF_DEBUGS
	debugprintf("lfm_start: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad6:
	u_close(lfd) ;

bad5:
bad1:
	if (op->lockfname != NULL) {
	    uc_free(op->lockfname) ;
	    op->lockfname = NULL ;
	}

bad0:
	if (lcp != NULL) lcp->stat = rs ;
	goto ret0 ;
}
/* end subroutine (lfm_start) */


int lfm_finish(LFM *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("lfm_finish: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LFM_MAGIC) return SR_NOTOPEN ;

	if (op->lfd >= 0) {
	    rs1 = u_close(op->lfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->lfd = -1 ;
	}

	if (op->lockfname != NULL) {
	    if ((op->type >= LFM_TCREATE) && (op->lockfname[0] != '\0')) {
	        u_unlink(op->lockfname) ;
	    }
	    rs1 = uc_free(op->lockfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->lockfname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("lfm_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (lfm_finish) */


int lfm_setpoll(LFM *op,int tocheck)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LFM_MAGIC) return SR_NOTOPEN ;

	op->tocheck = tocheck ;
	return SR_OK ;
}
/* end subroutine (lfm_setpoll) */


int lfm_getinfo(LFM *op,LFM_INFO *ip)
{

	if (op == NULL) return SR_FAULT ;
	if (ip == NULL) return SR_FAULT ;

	if (op->magic != LFM_MAGIC) return SR_NOTOPEN ;

	if (op->lockfname == NULL) return SR_NOANODE ;

	ip->dev = op->dev ;
	ip->ino = op->ino ;
	ip->tocheck = op->tocheck ;
	return SR_OK ;
}
/* end subroutine (lfm_getinfo) */


/* check that we still own the lock on the file */
int lfm_check(LFM *op,LFM_CHECK *cip,time_t daytime)
{
	struct ustat	sb ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("lfm_check: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LFM_MAGIC) return SR_NOTOPEN ;

	if (op->lockfname == NULL) return SR_NOANODE ;

#if	CF_DEBUGS
	debugprintf("lfm_check: continuing\n") ;
#endif

	if (cip != NULL) check_init(cip) ;

	if (op->tocheck < 0)
	    goto ret0 ;

	if (daytime == 0)
	    daytime = time(NULL) ;

	if ((daytime - op->ti_check) < op->tocheck)
	    goto ret0 ;

	op->ti_check = daytime ;
	op->pid_lock = -1 ;

/* do a check on the INODE (if it is the same) */

	if (((daytime - op->ti_stat) >= TO_MINSTAT) ||
	    (op->type == LFM_TCREATE)) {

	    rs = u_stat(op->lockfname,&sb) ;

	    if ((rs >= 0) && ((sb.st_dev != op->dev) ||
	        (sb.st_ino != op->ino))) rs = SR_LOCKLOST ;

	    op->ti_stat = daytime ;
	} /* end if (stat check) */

#if	CF_DEBUGS
	debugprintf("lfm_check: past INODE check\n") ;
#endif

	if (rs >= 0) {
	    if ((rs = lfm_lockbegin(op)) >= 0) {
	        pid_t	pid_lock ;
	        int	f_open = (rs > 0) ;

	        rs = lfm_lockreadpid(op) ;
	        pid_lock = rs ;

	        if ((rs >= 0) && (pid_lock != op->pid))  {
		    rs = SR_LOCKLOST ;
		}

	        if (rs >= 0)
	            rs = lfm_lockwritedate(op,daytime) ;

	        if (f_open) lfm_lockend(op) ;
	    } /* end if (open) */
	} /* end if (ok) */

	if (rs < 0) goto locklost ;

ret0:

#if	CF_DEBUGS
	debugprintf("lfm_check: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* the lock was lost */
locklost:
	if (cip != NULL) {
	    lfm_lockload(op,cip) ;
	    cip->stat = rs ;
	}

	rs = SR_LOCKLOST ;
	goto ret0 ;
}
/* end subroutine (lfm_check) */


/* 'printf'-like routine */
int lfm_printf(LFM *op,cchar *fmt,...)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	char		buf[BUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("lfm_printf: ent\n") ;
#endif /* CF_DEBUGS */

	if (op == NULL) rs = SR_FAULT ;

	if (op->lockfname == NULL) return SR_NOANODE ;

	if (op->lfd < 0) {
	    rs = u_open(op->lockfname,O_RDWR,0666) ;
	    if (rs >= 0) op->lfd = rs ;
	}

	if (rs >= 0) {

/* seek up to the proper place to start writing (for the user stuff) */

	    u_seek(op->lfd,op->owrite,SEEK_SET) ;

#if	CF_DEBUGS
	    debugprintf("lfm_printf: we seeked to %lld\n",op->owrite) ;
#endif /* CF_DEBUGS */

/* format the user's data (possibly write some out) */

	    {
	        va_list	ap ;
	        va_begin(ap,fmt) ;
	        rs = format(buf,BUFLEN,0,fmt,ap) ;
	        len = rs ;
	        va_end(ap) ;
	    }

	    if (rs >= 0) {
	        if ((rs = u_write(op->lfd,buf,len)) >= 0) {
	            op->owrite += len ;
	        }
	    }

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("lfm_printf: ret rs=%d len=%u\n",rs,len) ;
#endif /* CF_DEBUGS */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (lfm_printf) */


int lfm_rewind(LFM *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LFM_MAGIC) return SR_NOTOPEN ;

	if (op->lockfname == NULL) return SR_NOANODE ;

	op->owrite = op->orewind ;
	return SR_OK ;
}
/* end subroutine (lfm_rewind) */


int lfm_flush(LFM *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LFM_MAGIC) return SR_NOTOPEN ;

	if (op->lockfname == NULL) return SR_NOANODE ;

/* nothing to do now! */

	return rs ;
}
/* end subroutine (lfm_flush) */


/* get the PID of the guy who stole our lock! */
int lfm_getpid(LFM *op,pid_t *rp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->magic != LFM_MAGIC) return SR_NOTOPEN ;

	if (op->lockfname == NULL) return SR_NOANODE ;

	if (rp != NULL) {
	    *rp = op->pid ;
	}

	rs = (op->pid & INT_MAX) ;
	return rs ;
}
/* end subroutine (lfm_getpid) */


/* private subroutines */


/* try to load up information on the lock (lost) */
static int lfm_lockload(LFM *op,LFM_CHECK *lcp)
{
	int		rs = SR_OK ;

	if (lcp != NULL) {
	    const mode_t	omode = 0666 ;
	    const int		llen = LINEBUFLEN ;
	    const int		of = O_RDONLY ;
	    int			v ;

	    check_init(lcp) ;

	    if ((rs = u_open(op->lockfname,of,omode)) >= 0) {
	        FILEBUF	b ;
	        int		lfd = rs ;

	        if ((rs = filebuf_start(&b,lfd,0L,512,0)) >= 0) {
	            int	len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            if ((rs = filebuf_readline(&b,lbuf,llen,0)) > 0) {
	                len = rs ;

	                if ((len > 0) && (lbuf[len - 1] == '\n'))
	                    len -= 1 ;

	                op->pid_lock = (pid_t) -1 ;
	                if (cfdeci(lbuf,len,&v) >= 0)
	                    op->pid_lock = (pid_t) v ;

#if	CF_DEBUGS
	                debugprintf("lfm_check: pid=%d lockpid=%d\n",
	                    op->pid,op->pid_lock) ;
#endif

/* resume with the rest of the data gathering */

	                rs = lfm_locklost(op,lcp,&b) ;

	            } else
	                rs = SR_LOCKLOST ;

	            filebuf_finish(&b) ;
	        } /* end if (filebuf) */

	        u_close(lfd) ;
	    } /* end if (opened file) */

	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (lfm_lockload) */


/* read the lock information on a failure */
static int lfm_locklost(LFM *op,LFM_CHECK *lcp,FILEBUF *fp)
{
	STOREITEM	cb ;
	const int	buflen = LFM_CHECKBUFLEN ;
	int		rs ;
	int		rs1 ;

/* sanity */

	if (lcp == NULL) return SR_NOANODE ;
	if (fp == NULL) return SR_NOANODE ;

/* ok */

	lcp->pid = op->pid_lock ;
	if ((rs = storeitem_start(&cb,lcp->buf,buflen)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		sl, cl ;
	    const char	*sp, *cp, *ip ;
	    const char	*tp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    if ((rs = filebuf_readline(fp,lbuf,llen,0)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;

/* is there a nodename? */

	        sp = lbuf ;
	        sl = len ;
	        if ((tp = strnchr(sp,sl,'!')) != NULL) {

	            cl = sfshrink(lbuf,(tp - sp),&cp) ;

	            rs1 = storeitem_strw(&cb,cp,cl,&ip) ;

	            if (rs1 >= 0)
	                lcp->nodename = ip ;

	            sl = len - ((tp + 1) - sp) ;
	            sp = (tp + 1) ;

	        } /* end if (had a nodename) */

	        cl = sfshrink(sp,sl,&cp) ;

	        rs1 = storeitem_strw(&cb,cp,cl,&ip) ;

	        if (rs1 >= 0)
	            lcp->username = ip ;

	    } /* end if (reading node!user) */

	    if ((rs >= 0) && (len > 0)) {

	        if ((rs = filebuf_readline(fp,lbuf,llen,0)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            tp = strnpbrk(lbuf,len," \t") ;

	            sp = (tp + 1) ;
	            sl = len - ((tp + 1) - lbuf) ;
	            cl = sfshrink(sp,sl,&cp) ;

	            rs1 = storeitem_strw(&cb,cp,cl,&ip) ;

	            if (rs1 >= 0)
	                lcp->banner = ip ;

	        } /* end if (reading time-banner) */

	    } /* end if (reading continued) */

	    rs1 = storeitem_finish(&cb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (STOREITEM) */

	return rs ;
}
/* end subroutine (lfm_locklost) */


static int lfm_lockwrite(LFM *op,LFM_LOCKINFO *lip,int lfd)
{
	int		rs ;

	if ((rs = lfm_lockwriter(op,lip,lfd)) >= 0) {
	    offset_t	woff = rs ;
	    if ((rs = uc_ftruncate(lfd,woff)) >= 0) {
	        op->owrite = woff ;
	        op->orewind = woff ;
	        rs = (woff & INT_MAX) ;
	    }
	}

	return rs ;
}
/* end subroutine (lfm_lockwrite) */


static int lfm_lockwriter(LFM *op,LFM_LOCKINFO *lip,int lfd)
{
	FILEBUF		b ;
	int		rs ;
	int		rs1 ;
	int		woff = 0 ;

	if ((rs = filebuf_start(&b,lfd,0L,512,0)) >= 0) {

	    if (rs >= 0) {
	        rs = filebuf_printf(&b,"%u\n",op->pid) ;
	        woff += rs ;
	    }

	    if (rs >= 0) {
	        rs = filebuf_printf(&b,"%s!%s\n",lip->nn,lip->un) ;
	        woff += rs ;
	    }

	    if (rs >= 0) {
	        const char	*bn = (lip->bn != NULL) ? lip->bn : "" ;
	        char		timebuf[TIMEBUFLEN+1] ;
	        op->odate = woff ;
	        timestr_logz(lip->daytime,timebuf) ;
	        rs = filebuf_printf(&b,"%s %s\n",timebuf,bn) ;
	        woff += rs ;
	    }

	    rs1 = filebuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */

	return (rs >= 0) ? woff : rs ;
}
/* end subroutine (lfm_lockwriter) */


static int lfm_lockreadpid(LFM *op)
{
	const int	lfd = op->lfd ;
	int		rs ;
	int		v = 0 ;

	if ((rs = u_rewind(lfd)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN+1] ;
	    if ((rs = u_read(lfd,lbuf,llen)) >= 0) {
	        const char	*tp ;
	        int		len = rs ;

	        if ((tp = strnchr(lbuf,len,'\n')) != NULL)
	            lbuf[tp-lbuf] = '\0' ;

	        rs = cfdeci(lbuf,len,&v) ;
	        v &= INT_MAX ;

	    } /* end if */
	} /* end if (u_rewind) */

	return (rs >= 0) ? v : rs ;
}
/* end subroutine (lfm_lockreadpid) */


static int lfm_lockwritedate(LFM *op,time_t daytime)
{
	int		rs ;

	if ((rs = u_seek(op->lfd,op->odate,SEEK_SET)) >= 0) {
	    int		tl ;
	    char	timebuf[TIMEBUFLEN+2] ;
	    timestr_logz(daytime,timebuf) ;
	    tl = strlen(timebuf) ;
	    rs = u_write(op->lfd,timebuf,tl) ;
	}

	return rs ;
}
/* end subroutine (lfm_lockwritedate) */


static int lfm_lockbegin(LFM *op)
{
	int		rs = SR_OK ;
	int		f_open = FALSE ;

	if (op->lfd < 0) {
	    const mode_t	om = 0666 ;
	    const int		of = O_RDWR ;
	    f_open = TRUE ;
	    rs = u_open(op->lockfname,of,om) ;
	    op->lfd = rs ;
	}

	return (rs >= 0) ? f_open : rs ;
}
/* end subroutine (lfm_lockbegin) */


static int lfm_lockend(LFM *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_open = FALSE ;

	if (op->lfd >= 0) {
	    f_open = TRUE ; /* was *previosuly* open */
	    rs1 = u_close(op->lfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->lfd = -1 ;
	}

	return (rs >= 0) ? f_open : rs ;
}
/* end subroutine (lfm_lockend) */


static int check_init(LFM_CHECK *lcp)
{

	if (lcp == NULL) return SR_FAULT ;

#ifdef	OPTIONAL
	memset(lcp,0,sizeof(LFM_CHECK)) ;
#endif

	lcp->nodename = NULL ;
	lcp->username = NULL ;
	lcp->banner = NULL ;
	lcp->pid = -1 ;
	lcp->stat = 0 ;
	lcp->buf[0] = '\0' ;
	return SR_OK ;
}
/* end subroutine (check_init) */


