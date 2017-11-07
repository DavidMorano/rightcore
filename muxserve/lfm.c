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


	= Notes:

	Lock types:

	LFM_TRECORD		0		record lock
	LFM_TCREATE		1		create file 0444
	LFM_TEXCLUSIVE		2		exclusive open


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
extern int	sfnamecomp(cchar *,int,cchar **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	getnodename(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	format(char *,int,int,const char *,va_list) ;
extern int	isNotPresent(int) ;
extern int	isFailOpen(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
#endif

extern char	*getourenv(cchar **,cchar *) ;

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
	time_t		dt ;
} ;


/* forward references */

static int	lfm_startcheck(LFM *,time_t) ;
static int	lfm_startopen(LFM *,LFM_LOCKINFO *) ;

static int	lfm_lockload(LFM *,LFM_CHECK *) ;
static int	lfm_locklost(LFM *,LFM_CHECK *,FILEBUF *) ;
static int	lfm_lockwrite(LFM *,LFM_LOCKINFO *,int) ;
static int	lfm_lockwriter(LFM *,LFM_LOCKINFO *,int) ;
static int	lfm_lockwritedate(LFM *,time_t) ;
static int	lfm_lockreadpid(LFM *) ;
static int	lfm_lockbegin(LFM *op) ;
static int	lfm_lockend(LFM *op) ;
static int	lfm_checklock(LFM *,time_t) ;
static int	lfm_ourdevino(LFM *,USTAT *) ;

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
	const time_t	dt = time(NULL) ;
	const int	nnlen = NODENAMELEN ;
	const int	unlen = USERNAMELEN ;
	int		rs = SR_OK ;
	char		nnbuf[NODENAMELEN + 1] ;
	char		unbuf[USERNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("lfm_start: ent fname=%s\n",fname) ;
	debugprintf("lfm_start: type=%u\n",type) ;
#endif

	if (to < 0) to = LFM_TOLOCK ;

	memset(op,0,sizeof(LFM)) ;

/* argument check */

	if (fname[0] == '\0') return SR_INVALID ;

	if ((type < 0) || (type >= LFM_TOVERLAST))
	    return SR_INVALID ;

/* priliminary initialization */

	op->type = type ;
	op->lfd = -1 ;
	op->pid_lock = -1 ;
	op->tocheck = LFM_TOMINCHECK ;
	op->tolock = to ;
	op->pid = ugetpid() ;

	if (lcp != NULL) check_init(lcp) ;

/* continue itialization */

	if ((rs >= 0) && (nn == NULL)) {
	    nn = nnbuf ;
	    rs = getnodename(nnbuf,nnlen) ;
	}

	if ((rs >= 0) && (un == NULL)) {
	    un = unbuf ;
	    rs = getusername(unbuf,unlen,-1) ;
	}

	if (rs >= 0) {
	    int		fnl ;
	    cchar	*fnp ;
	    if ((fnl = sfnamecomp(fname,-1,&fnp)) > 0) {
	        cchar	*cp ;
	        if ((rs = uc_mallocstrw(fnp,fnl,&cp)) >= 0) {
	            const int	fnlen = (rs-1) ;
	            int		dnl ;
	            cchar	*dnp ;
	            op->lfname = cp ;
	            if ((dnl = sfdirname(op->lfname,fnlen,&dnp)) > 0) {
	                char	dbuf[MAXPATHLEN+1] ;
	                if ((rs = mkpath1w(dbuf,dnp,dnl)) >= 0) {
	                    LFM_LOCKINFO	li ;
	                    const int		am = (X_OK | W_OK) ;
	                    const int		rsn = SR_NOENT ;
	                    li.dt = dt ;
	                    li.nn = nn ;
	                    li.un = un ;
	                    li.bn = bn ;
	                    if ((rs = u_access(dbuf,am)) >= 0) {
	                        if ((rs = lfm_startcheck(op,dt)) >= 0) {
	                            rs = lfm_startopen(op,&li) ;
	                        }
	                    } else if (rs == rsn) {
	                        const mode_t	dm = 0777 ;
	                        if ((rs = mkdirs(dbuf,dm)) >= 0) {
	                            rs = lfm_startopen(op,&li) ;
	                        }
	                    }
#if	CF_DEBUGS
			    debugprintf("lfm_start: u_access() rs=%d\n",rs) ;
#endif
	                } /* end if (mkpath) */
	            }
	            if (rs < 0) {
	                if (op->lfname != NULL) {
	                    uc_free(op->lfname) ;
	                    op->lfname = NULL ;
	                }
	            }
	        } /* end if (m-a) */
	    } /* end if (sfnamecomp) */
	} /* end if (ok) */

	if (rs < 0) {
	    if (lcp != NULL) lfm_lockload(op,lcp) ;
	}

#if	CF_DEBUGS
	debugprintf("lfm_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
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

	if (op->lfname != NULL) {
	    if ((op->type >= LFM_TCREATE) && (op->lfname[0] != '\0')) {
	        u_unlink(op->lfname) ;
	    }
	    rs1 = uc_free(op->lfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->lfname = NULL ;
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

	if (op->lfname == NULL) return SR_NOANODE ;

	ip->dev = op->dev ;
	ip->ino = op->ino ;
	ip->tocheck = op->tocheck ;
	return SR_OK ;
}
/* end subroutine (lfm_getinfo) */


/* check that we still own the lock on the file */
int lfm_check(LFM *op,LFM_CHECK *cip,time_t dt)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("lfm_check: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LFM_MAGIC) return SR_NOTOPEN ;

	if (op->lfname == NULL) return SR_NOANODE ;

#if	CF_DEBUGS
	debugprintf("lfm_check: continuing\n") ;
#endif

	if (cip != NULL) check_init(cip) ;

	if (op->tocheck >= 0) {
	    if (dt == 0) dt = time(NULL) ;
	    if ((dt - op->ti_check) >= op->tocheck) {
	        op->ti_check = dt ;
	        op->pid_lock = -1 ;

/* do a check on the INODE (if it is the same) */
	        if (((dt - op->ti_stat) >= LFM_TOMINSTAT) ||
	            (op->type == LFM_TCREATE)) {
	            USTAT	sb ;

	            if ((rs = u_stat(op->lfname,&sb)) >= 0) {
	                op->ti_stat = dt ;
	                rs = lfm_ourdevino(op,&sb) ;
	            }
	        } /* end if (stat check) */

#if	CF_DEBUGS
	        debugprintf("lfm_check: past INODE check\n") ;
#endif

	        if (rs >= 0) {
	            rs = lfm_checklock(op,dt) ;
	        } /* end if (ok) */


	        if ((rs < 0) && (cip != NULL)) {
	            lfm_lockload(op,cip) ;
	            cip->stat = rs ;
	        }

	    } /* end if */
	} /* end if (time-out enabled) */

#if	CF_DEBUGS
	debugprintf("lfm_check: ret rs=%d\n",rs) ;
#endif

	return rs ;
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

	if (op->lfname == NULL) return SR_NOANODE ;

	if (op->lfd < 0) {
	    rs = u_open(op->lfname,O_RDWR,0666) ;
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

	if (op->lfname == NULL) return SR_NOANODE ;

	op->owrite = op->orewind ;
	return SR_OK ;
}
/* end subroutine (lfm_rewind) */


int lfm_flush(LFM *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LFM_MAGIC) return SR_NOTOPEN ;

	if (op->lfname == NULL) return SR_NOANODE ;

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

	if (op->lfname == NULL) return SR_NOANODE ;

	if (rp != NULL) {
	    *rp = op->pid ;
	}

	rs = (op->pid & INT_MAX) ;
	return rs ;
}
/* end subroutine (lfm_getpid) */


/* private subroutines */


static int lfm_startcheck(LFM *op,time_t dt)
{
	USTAT		sb ;
	const int	type = op->type ;
	int		rs ;
	cchar		*lfn = op->lfname ;
	if ((rs = u_stat(lfn,&sb)) >= 0) {
	    if (S_ISREG(sb.st_mode)) {
	        const int	to = op->tolock ;
	        op->ti_check = dt ;
	        op->ti_stat = dt ;
	        if (type >= LFM_TCREATE) {
	            if ((dt - sb.st_mtime) >= to) {
	                u_unlink(op->lfname) ;
	            } else {
	                rs = SR_LOCKED ;
	            }
	        } /* end if (good stat) */
	    } else {
	        rs = SR_ISDIR ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	} /* end if (u_stat) */
#if	CF_DEBUGS
	debugprintf("lfm_startcheck: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lfm_startcheck) */


static int lfm_startopen(LFM *op,LFM_LOCKINFO *lip)
{
	const int	type = op->type ;
	int		rs ;
	int		oflags = (O_RDWR | O_CREAT) ;
	mode_t		omode = 0664 ;

#if	CF_DEBUGS
	debugprintf("lfm_startopen: ent lfn=%s\n",op->lfname) ;
	debugprintf("lfm_startopen: ent type=%u\n",type) ;
#endif

	if (type >= LFM_TCREATE) omode = 0444 ;
	if (type >= LFM_TEXCLUSIVE) oflags |= O_EXCL ;
	if ((rs = u_open(op->lfname,oflags,omode)) >= 0) {
	    const int	lfd = rs ;

#if	CF_DEBUGS
	    debugprintf("lfm_startopen: u_open() rs=%d\n",rs) ;
#endif

	    if ((rs = uc_lockf(lfd,F_TLOCK,0L)) >= 0) {
	        if ((rs = lfm_lockwrite(op,lip,lfd)) >= 0) {
	            USTAT	sb ;
	            if ((rs = u_fstat(lfd,&sb)) >= 0) {
	                op->lfd = lfd ;
	                op->magic = LFM_MAGIC ;
	                op->dev = sb.st_dev ;
	                op->ino = sb.st_ino ;
	            }
	        }
	    } else if (rs == SR_ACCESS) {
	        rs = SR_LOCKED ;
	    } /* end if (uc_lockf) */

	    if (rs < 0) {
	        switch (rs) {
	        case SR_EXIST:
	        case SR_ACCESS:
	            rs = SR_LOCKED ;
	            break ;
	        } /* end switch */
	    } /* end if */
	} /* end if (u_open) */

#if	CF_DEBUGS
	debugprintf("lfm_startopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutie (lfm_startopen) */


/* try to load up information on the lock (lost) */
static int lfm_lockload(LFM *op,LFM_CHECK *lcp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lcp != NULL) {
	    const mode_t	omode = 0666 ;
	    const int		llen = LINEBUFLEN ;
	    const int		of = O_RDONLY ;
	    int			v ;

	    check_init(lcp) ;

	    if ((rs = u_open(op->lfname,of,omode)) >= 0) {
	        FILEBUF		b ;
	        const int	lfd = rs ;

	        if ((rs = filebuf_start(&b,lfd,0L,512,0)) >= 0) {
	            int		len ;
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

	            } else {
	                rs = SR_LOCKLOST ;
	            }

	            rs1 = filebuf_finish(&b) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (filebuf) */

	        rs1 = u_close(lfd) ;
	        if (rs >= 0) rs = rs1 ;
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

	            if ((tp = strnpbrk(lbuf,len," \t")) != NULL) {
	                sp = (tp + 1) ;
	                sl = len - ((tp + 1) - lbuf) ;
	                if ((cl = sfshrink(sp,sl,&cp)) >= 0) {
	                    if ((rs = storeitem_strw(&cb,cp,cl,&ip)) >= 0) {
	                        lcp->banner = ip ;
	                    }
	                }
	            }

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
	        char		tbuf[TIMEBUFLEN+1] ;
	        op->odate = woff ;
	        timestr_logz(lip->dt,tbuf) ;
	        rs = filebuf_printf(&b,"%s %s\n",tbuf,bn) ;
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

	        if ((tp = strnchr(lbuf,len,'\n')) != NULL) {
	            lbuf[tp-lbuf] = '\0' ;
	        }

	        rs = cfdeci(lbuf,len,&v) ;
	        v &= INT_MAX ;

	    } /* end if */
	} /* end if (u_rewind) */

	return (rs >= 0) ? v : rs ;
}
/* end subroutine (lfm_lockreadpid) */


static int lfm_lockwritedate(LFM *op,time_t dt)
{
	int		rs ;

	if ((rs = u_seek(op->lfd,op->odate,SEEK_SET)) >= 0) {
	    int		tl ;
	    char	tbuf[TIMEBUFLEN+2] ;
	    timestr_logz(dt,tbuf) ;
	    tl = strlen(tbuf) ;
	    rs = u_write(op->lfd,tbuf,tl) ;
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
	    rs = u_open(op->lfname,of,om) ;
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


static int lfm_checklock(LFM *op,time_t dt)
{
	int		rs ;
	int		v = 0 ;
	if ((rs = lfm_lockbegin(op)) >= 0) {
	    int	f_open = (rs > 0) ;

	    if ((rs = lfm_lockreadpid(op)) >= 0) {
	        pid_t pid_lock = rs ;

	        if (pid_lock != op->pid)  {
	            rs = SR_LOCKLOST ;
	        }

	        if (rs >= 0) {
	            v = pid_lock ;
	            rs = lfm_lockwritedate(op,dt) ;
	        }

	    } /* end if (lfm_lockreadpid) */
	    if (f_open) lfm_lockend(op) ;
	} /* end if (lfm_lock) */
	return (rs >= 0) ? v : rs ;
}
/* end subroutine (lfm_checklock) */


static int lfm_ourdevino(LFM *op,USTAT *sbp)
{
	int		rs = SR_OK ;
	if ((sbp->st_dev != op->dev) || (sbp->st_ino != op->ino)) {
	    rs = SR_LOCKLOST ;
	}
	return rs ;
}
/* end subroutine (lfm_ourdevino) */


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


